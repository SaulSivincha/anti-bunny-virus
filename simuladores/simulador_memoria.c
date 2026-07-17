#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    enum { BLOQUE_MB = 32, REPETICIONES = 10 };
    void *bloques[REPETICIONES] = {0};
    for (int i = 0; i < REPETICIONES; ++i) {
        bloques[i] = malloc((size_t)BLOQUE_MB * 1024 * 1024);
        if (bloques[i] == NULL) break;
        memset(bloques[i], 0, (size_t)BLOQUE_MB * 1024 * 1024);
        printf("Memoria simulada: %d MB\n", (i + 1) * BLOQUE_MB);
        sleep(1);
    }
    sleep(5);
    for (int i = 0; i < REPETICIONES; ++i) free(bloques[i]);
    return 0;
}
