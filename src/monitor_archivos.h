#ifndef MONITOR_ARCHIVOS_H
#define MONITOR_ARCHIVOS_H

typedef struct {
    char ruta[512];
    long tamano_bytes;
    float crecimiento_mb_s;
} ArchivoInfo;

int escanear_archivos(const char* directorio, float intervalo, ArchivoInfo* lista, int max_archivos);

#endif