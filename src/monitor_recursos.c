#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "monitor_recursos.h"

#define MAX_HISTORIAL_RECURSOS 4096

typedef struct {
    int pid;
    unsigned long long ticks;
} MuestraAnterior;

static MuestraAnterior historial[MAX_HISTORIAL_RECURSOS];
static int total_historial;
static struct timespec instante_anterior;
static int hay_muestra_anterior;

static int es_pid(const char *nombre) {
    if (*nombre == '\0') return 0;
    for (; *nombre != '\0'; ++nombre) if (!isdigit((unsigned char)*nombre)) return 0;
    return 1;
}

static unsigned long long ticks_anteriores(int pid) {
    for (int i = 0; i < total_historial; ++i) {
        if (historial[i].pid == pid) return historial[i].ticks;
    }
    return 0;
}

static int leer_recurso(int pid, RecursoInfo *recurso, unsigned long long *ticks) {
    char ruta[PATH_MAX];
    char linea[4096];
    char *cierre;
    FILE *archivo;
    unsigned long long utime, stime;
    long rss_kb = 0;

    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL || fgets(linea, sizeof(linea), archivo) == NULL) {
        if (archivo != NULL) fclose(archivo);
        return -1;
    }
    fclose(archivo);
    cierre = strrchr(linea, ')');
    if (cierre == NULL ||
        sscanf(cierre + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu",
               &utime, &stime) != 2) return -1;

    snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL) return -1;
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        if (sscanf(linea, "Name: %255s", recurso->nombre) == 1) continue;
        if (sscanf(linea, "VmRSS: %ld kB", &rss_kb) == 1) break;
    }
    fclose(archivo);

    recurso->pid = pid;
    recurso->memoria_mb = (float)rss_kb / 1024.0f;
    *ticks = utime + stime;
    return 0;
}

int obtener_recursos(RecursoInfo *lista, int max_procesos) {
    DIR *proc = opendir("/proc");
    struct dirent *entrada;
    struct timespec ahora;
    MuestraAnterior nuevo_historial[MAX_HISTORIAL_RECURSOS];
    long ticks_por_segundo = sysconf(_SC_CLK_TCK);
    double transcurrido = 0.0;
    int total = 0;

    if (proc == NULL || lista == NULL || max_procesos <= 0 || ticks_por_segundo <= 0) return 0;
    clock_gettime(CLOCK_MONOTONIC, &ahora);
    if (hay_muestra_anterior) {
        transcurrido = (double)(ahora.tv_sec - instante_anterior.tv_sec) +
                      (double)(ahora.tv_nsec - instante_anterior.tv_nsec) / 1000000000.0;
    }

    while ((entrada = readdir(proc)) != NULL && total < max_procesos && total < MAX_HISTORIAL_RECURSOS) {
        unsigned long long ticks;
        if (!es_pid(entrada->d_name)) continue;
        if (leer_recurso(atoi(entrada->d_name), &lista[total], &ticks) != 0) continue;
        if (transcurrido > 0.0) {
            unsigned long long previo = ticks_anteriores(lista[total].pid);
            lista[total].cpu_porcentaje = ticks >= previo
                ? (float)(((double)(ticks - previo) / (double)ticks_por_segundo) / transcurrido * 100.0)
                : 0.0f;
        } else {
            lista[total].cpu_porcentaje = 0.0f;
        }
        nuevo_historial[total].pid = lista[total].pid;
        nuevo_historial[total].ticks = ticks;
        ++total;
    }
    closedir(proc);
    memcpy(historial, nuevo_historial, (size_t)total * sizeof(MuestraAnterior));
    total_historial = total;
    instante_anterior = ahora;
    hay_muestra_anterior = 1;
    return total;
}
