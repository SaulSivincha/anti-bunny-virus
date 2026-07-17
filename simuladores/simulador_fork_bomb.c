#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Proliferación recursiva acotada: se usa solo para validar el monitor. */
enum { LIMITE_PROCESOS = 64, RAMAS_POR_PROCESO = 8 };

static void proliferar(atomic_int *creados) {
    pid_t hijos[RAMAS_POR_PROCESO];
    int total_hijos = 0;
    for (int i = 0; i < RAMAS_POR_PROCESO; ++i) {
        int indice = atomic_fetch_add(creados, 1);
        if (indice >= LIMITE_PROCESOS - 1) break;
        pid_t hijo = fork();
        if (hijo == 0) {
            struct timespec espera = {.tv_sec = 0, .tv_nsec = 10000000};
            nanosleep(&espera, NULL);
            proliferar(creados);
            _exit(0);
        }
        if (hijo > 0) hijos[total_hijos++] = hijo;
    }
    sleep(5);
    for (int i = 0; i < total_hijos; ++i) waitpid(hijos[i], NULL, 0);
}

int main(void) {
    atomic_int *creados = mmap(NULL, sizeof(*creados), PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (creados == MAP_FAILED) return 1;
    atomic_init(creados, 1);
    const char *autorizacion = getenv("ANTI_BUNNY_AUTH_FILE");
    if (autorizacion != NULL) {
        FILE *archivo = fopen(autorizacion, "w");
        if (archivo != NULL) { fprintf(archivo, "%d\n", (int)getpgrp()); fclose(archivo); }
    }
    printf("Simulador recursivo acotado: hasta %d procesos; PGID=%d\n", LIMITE_PROCESOS, (int)getpgrp());
    proliferar(creados);
    munmap(creados, sizeof(*creados));
    return 0;
}
