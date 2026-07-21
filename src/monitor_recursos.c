/*
 * Recolector de recursos. Lee ticks de CPU y VmRSS desde /proc y compara dos
 * muestras de la misma identidad PID-starttime para calcular tasas temporales.
 */
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "monitor_recursos.h"

/* Historial de CPU y RSS entre ciclos para calcular tasas por segundo. */
#define MAX_HISTORIAL_RECURSOS 8192
// Conserva los acumulados necesarios para calcular diferencias entre ciclos.
typedef struct { int pid; unsigned long long starttime, ticks; float memoria_mb; } MuestraAnterior;
static MuestraAnterior historial[MAX_HISTORIAL_RECURSOS];
static int total_historial;
static struct timespec instante_anterior;
static int hay_muestra_anterior;

static int es_pid(const char *n) { if (*n == '\0') return 0; for (; *n; ++n) if (!isdigit((unsigned char)*n)) return 0; return 1; }

/* Función para buscar la muestra previa de un proceso identificado por pid y starttime. */
static const MuestraAnterior *previo(int pid, unsigned long long starttime) {
    for (int i = 0; i < total_historial; ++i) if (historial[i].pid == pid && historial[i].starttime == starttime) return &historial[i];
    return NULL;
}
/* Función para leer ticks de CPU, starttime y memoria RSS de /proc/pid. */
static int leer_recurso(int pid, RecursoInfo *recurso, unsigned long long *ticks) {
    char ruta[PATH_MAX], linea[4096], *cierre, *guardar = NULL, *token;
    FILE *archivo; int indice = 0; long rss_kb = 0;
    memset(recurso, 0, sizeof(*recurso));
    // stat entrega ticks de usuario/kernel y el tiempo de inicio del proceso.
    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL || fgets(linea, sizeof(linea), archivo) == NULL) { if (archivo) fclose(archivo); return -1; }
    fclose(archivo); cierre = strrchr(linea, ')'); if (cierre == NULL) return -1;
    token = strtok_r(cierre + 2, " ", &guardar);
    while (token != NULL) { if (indice == 11) *ticks = strtoull(token, NULL, 10); if (indice == 12) *ticks += strtoull(token, NULL, 10); if (indice == 19) recurso->starttime = strtoull(token, NULL, 10); token = strtok_r(NULL, " ", &guardar); ++indice; }
    if (indice <= 19) return -1;
    // status aporta el nombre y la memoria residente VmRSS.
    snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid); archivo = fopen(ruta, "r"); if (archivo == NULL) return -1;
    while (fgets(linea, sizeof(linea), archivo) != NULL) { if (sscanf(linea, "Name: %255s", recurso->nombre) == 1) continue; if (sscanf(linea, "VmRSS: %ld kB", &rss_kb) == 1) break; }
    fclose(archivo); recurso->pid = pid; recurso->memoria_mb = (float)rss_kb / 1024.0f; return 0;
}
/*
 * Función para recorrer /proc y calcular porcentaje de CPU
 * y velocidad de crecimiento de memoria respecto al ciclo anterior.
 */
int obtener_recursos(RecursoInfo *lista, int max_procesos) {
    DIR *proc = opendir("/proc");
    struct dirent *entrada;
    struct timespec ahora;
    long hz = sysconf(_SC_CLK_TCK);
    double dt = 0.0;
    int total = 0;
    static unsigned long long ticks_actuales[MAX_HISTORIAL_RECURSOS];
    if (proc == NULL || lista == NULL || max_procesos <= 0 || hz <= 0) return 0;

    // Medir el intervalo real entre esta fotografía y la anterior.
    clock_gettime(CLOCK_MONOTONIC, &ahora);
    if (hay_muestra_anterior) {
        dt = (double)(ahora.tv_sec - instante_anterior.tv_sec) + 
             (double)(ahora.tv_nsec - instante_anterior.tv_nsec) / 1e9;
    }
    while ((entrada = readdir(proc)) != NULL && total < max_procesos && total < MAX_HISTORIAL_RECURSOS) {
        unsigned long long ticks = 0;
        const MuestraAnterior *p;
        
        if (!es_pid(entrada->d_name) || leer_recurso(atoi(entrada->d_name), &lista[total], &ticks) != 0) continue;
        
        p = previo(lista[total].pid, lista[total].starttime);
        if (p != NULL && dt > 0.0) {
            // Convertir ticks acumulados a porcentaje y diferencia de RSS a MB/s.
            lista[total].cpu_porcentaje = (float)(((double)(ticks - p->ticks) / hz) / dt * 100.0);
            lista[total].delta_memoria_mb_s = (lista[total].memoria_mb - p->memoria_mb) / (float)dt;
        }
        ticks_actuales[total] = ticks;
        ++total;
    }
    closedir(proc);
    // La muestra actual se convierte en referencia para el siguiente ciclo.
    total_historial = 0;
    for (int i = 0; i < total && total_historial < MAX_HISTORIAL_RECURSOS; ++i) {
        historial[total_historial] = (MuestraAnterior){
            .pid = lista[i].pid,
            .starttime = lista[i].starttime,
            .ticks = ticks_actuales[i],
            .memoria_mb = lista[i].memoria_mb
        };
        ++total_historial;
    }
    instante_anterior = ahora;
    hay_muestra_anterior = 1;
    return total;
}
