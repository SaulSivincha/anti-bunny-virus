#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Simulador de consumo gradual de memoria para probar alertas de recursos.
 * Reserva bloques, los toca para forzar RSS y libera al finalizar.
 */
int main(void) {
    enum { BLOQUE_MB = 32, REPETICIONES = 10 };
    volatile unsigned char *bloques[REPETICIONES] = {0};
    for (int i = 0; i < REPETICIONES; ++i) {
        size_t bytes = (size_t)BLOQUE_MB * 1024 * 1024;
        bloques[i] = malloc(bytes);
        if (bloques[i] == NULL) break;
        for (size_t pagina = 0; pagina < bytes; pagina += 4096) {
            bloques[i][pagina] = (unsigned char)i;
        }
        printf("Memoria simulada: %d MB\n", (i + 1) * BLOQUE_MB);
        sleep(1);
    }
    sleep(5);
    for (int i = 0; i < REPETICIONES; ++i) free((void *)bloques[i]);
    return 0;
}
