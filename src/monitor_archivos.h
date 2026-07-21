/*
 * Interfaz del monitor de archivos. Representa el crecimiento de una ruta y,
 * cuando es posible, el proceso que la mantiene abierta durante el muestreo.
 */
#ifndef MONITOR_ARCHIVOS_H
#define MONITOR_ARCHIVOS_H

#include <limits.h>
#include <time.h>

#include "monitor_procesos.h"

// Estado actual de un archivo y resultado de su atribución a un escritor.
typedef struct {
    char ruta[PATH_MAX];
    long long tamano_bytes;
    float crecimiento_mb_s;
    time_t mtime;
    int pid_escritor;
    int pgid_escritor;
    int atribuido;
} ArchivoInfo;

// Mide recursivamente los archivos regulares del directorio indicado.
int escanear_archivos(const char *directorio, float intervalo,
                      ArchivoInfo *lista, int max_archivos);
// Busca la ruta del archivo entre los descriptores abiertos de los procesos.
void atribuir_archivo(ArchivoInfo *archivo, const ProcesoInfo *procesos, int n_procesos);

#endif
