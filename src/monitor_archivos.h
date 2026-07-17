#ifndef MONITOR_ARCHIVOS_H
#define MONITOR_ARCHIVOS_H

#include <limits.h>

typedef struct {
    char ruta[PATH_MAX];
    long long tamano_bytes;
    float crecimiento_mb_s;
} ArchivoInfo;

int escanear_archivos(const char *directorio, float intervalo,
                      ArchivoInfo *lista, int max_archivos);

#endif
