#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "monitor_procesos.h"

#define MAX_HISTORIAL_PROCESOS 8192

typedef struct {
    int pid;
    unsigned long long starttime;
    int hijos;
} MuestraProceso;

static MuestraProceso historial[MAX_HISTORIAL_PROCESOS];
static int total_historial;
static struct timespec instante_anterior;
static int hay_muestra_anterior;

static int es_pid(const char *nombre) {
    if (*nombre == '\0') return 0;
    for (; *nombre != '\0'; ++nombre) if (!isdigit((unsigned char)*nombre)) return 0;
    return 1;
}

static int parsear_stat(char *linea, ProcesoInfo *proceso) {
    char *cierre = strrchr(linea, ')');
    char *guardar = NULL;
    char *token;
    int indice = 0;
    if (cierre == NULL || sscanf(linea, "%d (%255[^)])", &proceso->pid, proceso->nombre) != 2) return -1;
    token = strtok_r(cierre + 2, " ", &guardar);
    while (token != NULL) {
        if (indice == 1) proceso->ppid = atoi(token);
        else if (indice == 2) proceso->pgid = atoi(token);
        else if (indice == 19) proceso->starttime = strtoull(token, NULL, 10);
        token = strtok_r(NULL, " ", &guardar);
        ++indice;
    }
    return indice > 19 ? 0 : -1;
}

static int leer_proceso(int pid, ProcesoInfo *proceso) {
    char ruta_stat[PATH_MAX], ruta_exe[PATH_MAX], linea[4096];
    FILE *archivo;
    struct stat datos;
    struct passwd *usuario;
    ssize_t longitud;

    memset(proceso, 0, sizeof(*proceso));
    snprintf(ruta_stat, sizeof(ruta_stat), "/proc/%d/stat", pid);
    archivo = fopen(ruta_stat, "r");
    if (archivo == NULL || fgets(linea, sizeof(linea), archivo) == NULL) {
        if (archivo != NULL) fclose(archivo);
        return -1;
    }
    fclose(archivo);
    if (parsear_stat(linea, proceso) != 0) return -1;

    snprintf(ruta_stat, sizeof(ruta_stat), "/proc/%d", pid);
    if (stat(ruta_stat, &datos) == 0) {
        proceso->uid = (unsigned int)datos.st_uid;
        usuario = getpwuid(datos.st_uid);
        snprintf(proceso->usuario, sizeof(proceso->usuario), "%s", usuario != NULL ? usuario->pw_name : "desconocido");
    } else {
        snprintf(proceso->usuario, sizeof(proceso->usuario), "desconocido");
    }
    snprintf(ruta_exe, sizeof(ruta_exe), "/proc/%d/exe", pid);
    longitud = readlink(ruta_exe, proceso->ruta, sizeof(proceso->ruta) - 1);
    if (longitud >= 0) proceso->ruta[longitud] = '\0';
    return 0;
}

static int hijos_previos(const ProcesoInfo *proceso) {
    for (int i = 0; i < total_historial; ++i)
        if (historial[i].pid == proceso->pid && historial[i].starttime == proceso->starttime)
            return historial[i].hijos;
    return 0;
}

int obtener_procesos(ProcesoInfo *lista, int max_procesos) {
    DIR *proc = opendir("/proc");
    struct dirent *entrada;
    struct timespec ahora;
    double transcurrido = 0.0;
    int total = 0;

    if (proc == NULL || lista == NULL || max_procesos <= 0) return 0;
    clock_gettime(CLOCK_MONOTONIC, &ahora);
    if (hay_muestra_anterior) {
        transcurrido = (double)(ahora.tv_sec - instante_anterior.tv_sec) +
                      (double)(ahora.tv_nsec - instante_anterior.tv_nsec) / 1000000000.0;
    }
    while ((entrada = readdir(proc)) != NULL && total < max_procesos && total < MAX_HISTORIAL_PROCESOS) {
        if (es_pid(entrada->d_name) && leer_proceso(atoi(entrada->d_name), &lista[total]) == 0) ++total;
    }
    closedir(proc);
    for (int i = 0; i < total; ++i) {
        for (int j = 0; j < total; ++j) if (lista[j].ppid == lista[i].pid) ++lista[i].hijos;
        int previo = hijos_previos(&lista[i]);
        lista[i].hijos_nuevos = lista[i].hijos > previo ? lista[i].hijos - previo : 0;
        lista[i].hijos_nuevos_s = transcurrido > 0.0 ? (float)(lista[i].hijos_nuevos / transcurrido) : 0.0f;
        historial[i] = (MuestraProceso){lista[i].pid, lista[i].starttime, lista[i].hijos};
    }
    total_historial = total;
    instante_anterior = ahora;
    hay_muestra_anterior = 1;
    return total;
}
