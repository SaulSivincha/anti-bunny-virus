#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/*
 * Simulador de crecimiento de fichero en el directorio vigilado por el monitor.
 * Escribe bloques periódicos para generar bytes/s detectables.
 */
int main(void) {
    const char *directorio = "/tmp/anti_bunny_demo";
    const char *destino = "/tmp/anti_bunny_demo/archivo_crecimiento_demo.bin";
    const size_t bloque_bytes = 2 * 1024 * 1024;
    unsigned char *bloque = calloc(1, bloque_bytes);
    struct timespec espera = { .tv_sec = 0, .tv_nsec = 200000000 };
    if (bloque == NULL || (mkdir(directorio, 0755) != 0 && errno != EEXIST)) return 1;
    FILE *archivo = fopen(destino, "wb");
    if (archivo == NULL) return 1;
    for (int i = 0; i < 8; ++i) {
        fwrite(bloque, 1, bloque_bytes, archivo);
        fflush(archivo);
        printf("Escritura simulada: %d MB\n", (i + 1) * 2);
        nanosleep(&espera, NULL);
    }
    fclose(archivo);
    free(bloque);
    printf("Archivo de prueba generado en %s\n", destino);
    return 0;
}
