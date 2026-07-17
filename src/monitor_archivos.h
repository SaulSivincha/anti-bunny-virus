#ifndef MONITOR_ARCHIVOS_H
#define MONITOR_ARCHIVOS_H

#include <limits.h>
#include <time.h>

#include "monitor_procesos.h"

typedef struct {
    char ruta[PATH_MAX];
    long long tamano_bytes;
    float crecimiento_mb_s;
    time_t mtime;
    int pid_escritor;
    int pgid_escritor;
    int atribuido;
} ArchivoInfo;

int escanear_archivos(const char *directorio, float intervalo,
                      ArchivoInfo *lista, int max_archivos);
void atribuir_archivo(ArchivoInfo *archivo, const ProcesoInfo *procesos, int n_procesos);

#endif
