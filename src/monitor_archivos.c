#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "monitor_archivos.h"

#define MAX_ESTADOS_ARCHIVO 1024

typedef struct {
    char ruta[PATH_MAX];
    long long tamano;
} EstadoArchivo;

static EstadoArchivo estados[MAX_ESTADOS_ARCHIVO];
static int total_estados;

static long long tamano_anterior(const char *ruta, long long actual) {
    for (int i = 0; i < total_estados; ++i) {
        if (strcmp(estados[i].ruta, ruta) == 0) return estados[i].tamano;
    }
    return actual;
}

static void recordar_archivo(const char *ruta, long long tamano) {
    for (int i = 0; i < total_estados; ++i) {
        if (strcmp(estados[i].ruta, ruta) == 0) {
            estados[i].tamano = tamano;
            return;
        }
    }
    if (total_estados < MAX_ESTADOS_ARCHIVO) {
        snprintf(estados[total_estados].ruta, sizeof(estados[total_estados].ruta), "%s", ruta);
        estados[total_estados].tamano = tamano;
        ++total_estados;
    }
}

static void escanear_directorio(const char *directorio, float intervalo,
                                ArchivoInfo *lista, int max_archivos, int *total) {
    DIR *dir = opendir(directorio);
    struct dirent *entrada;
    if (dir == NULL) return;

    while ((entrada = readdir(dir)) != NULL && *total < max_archivos) {
        char ruta[PATH_MAX];
        struct stat datos;
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) continue;
        if (snprintf(ruta, sizeof(ruta), "%s/%s", directorio, entrada->d_name) >= (int)sizeof(ruta)) continue;
        if (lstat(ruta, &datos) != 0) continue;
        if (S_ISDIR(datos.st_mode)) {
            escanear_directorio(ruta, intervalo, lista, max_archivos, total);
        } else if (S_ISREG(datos.st_mode)) {
            long long anterior = tamano_anterior(ruta, (long long)datos.st_size);
            ArchivoInfo *archivo = &lista[*total];
            snprintf(archivo->ruta, sizeof(archivo->ruta), "%s", ruta);
            archivo->tamano_bytes = (long long)datos.st_size;
            archivo->crecimiento_mb_s = intervalo > 0.0f && archivo->tamano_bytes > anterior
                ? (float)(archivo->tamano_bytes - anterior) / (1024.0f * 1024.0f * intervalo)
                : 0.0f;
            recordar_archivo(ruta, archivo->tamano_bytes);
            ++*total;
        }
    }
    closedir(dir);
}

int escanear_archivos(const char *directorio, float intervalo,
                      ArchivoInfo *lista, int max_archivos) {
    int total = 0;
    if (directorio == NULL || lista == NULL || max_archivos <= 0) return 0;
    if (mkdir(directorio, 0755) != 0) {
        struct stat datos;
        if (stat(directorio, &datos) != 0 || !S_ISDIR(datos.st_mode)) return 0;
    }
    escanear_directorio(directorio, intervalo, lista, max_archivos, &total);
    return total;
}
