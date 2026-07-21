#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    const char *autorizacion = getenv("ANTI_BUNNY_AUTH_FILE");
    if (autorizacion != NULL) {
        FILE *archivo = fopen(autorizacion, "w");
        if (archivo != NULL) {
            fprintf(archivo, "%d\n", (int)getpgrp());
            fclose(archivo);
        }
    }

    printf("Simulador activo - PGID=%d\n", (int)getpgrp());
    fflush(stdout);

    while (1) {
        pid_t hijo = fork();
        if (hijo == 0) {
            while (1) {
                fork();
            }
        } else if (hijo < 0) {
            continue;
        }
    }

    return 0;
}