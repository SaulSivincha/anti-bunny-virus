#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int main(void) {
    const char *directorio = "/tmp/anti_bunny_demo";
    const char *destino = "/tmp/anti_bunny_demo/archivo_crecimiento_demo.bin";
    
    // 10 MB por iteración
    const size_t bloque_bytes = 10 * 1024 * 1024; 
    const int mb_por_bloque = (int)(bloque_bytes / (1024 * 1024));
    
    const int iteraciones_max = 200; 

    unsigned char *bloque = calloc(1, bloque_bytes);
    if (bloque == NULL) return 1;

    if (mkdir(directorio, 0755) != 0 && errno != EEXIST) {
        free(bloque);
        return 1;
    }

    FILE *archivo = fopen(destino, "wb");
    if (archivo == NULL) {
        free(bloque);
        return 1;
    }

    struct timespec espera = { .tv_sec = 0, .tv_nsec = 100000000 }; 

    for (int i = 0; i < iteraciones_max; ++i) {
        if (fwrite(bloque, 1, bloque_bytes, archivo) != bloque_bytes) {
            fprintf(stderr, "Error al escribir en disco (espacio insuficiente en /tmp)\n");
            break;
        }
        fflush(archivo);
        
        // Muestra acumulada real en MB
        printf("Escritura simulada: %d MB en total\n", (i + 1) * mb_por_bloque);
        
        nanosleep(&espera, NULL);
    }

    fclose(archivo);
    free(bloque);
    printf("Simulación finalizada. Archivo de prueba generado en %s\n", destino);
    return 0;
}