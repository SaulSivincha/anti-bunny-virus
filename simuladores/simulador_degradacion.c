#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t terminar = 0;

static void manejar_senal(int signo) {
    (void)signo;
    terminar = 1;
}

static long leer_entero(const char *nombre, long defecto, long minimo, long maximo) {
    const char *valor = getenv(nombre);
    char *fin = NULL;
    long numero;

    if (valor == NULL || *valor == '\0')
        return defecto;

    errno = 0;
    numero = strtol(valor, &fin, 10);

    if (errno != 0 || fin == valor || *fin != '\0' ||
        numero < minimo || numero > maximo) {
        fprintf(stderr,
                "Valor invalido para %s=%s; usando %ld\n",
                nombre, valor, defecto);
        return defecto;
    }

    return numero;
}

int main(void) {
    /*
     * Valores conservadores por defecto.
     * Para una corrida experimental se configuran mediante variables
     * de entorno sin recompilar el programa.
     */
    long objetivo = leer_entero(
        "DEGRADACION_PROCESOS", 128, 1, 5000
    );

    long lote = leer_entero(
        "DEGRADACION_LOTE", 8, 1, 256
    );

    long intervalo_ms = leer_entero(
        "DEGRADACION_INTERVALO_MS", 250, 10, 10000
    );

    long duracion_s = leer_entero(
        "DEGRADACION_DURACION_S", 30, 1, 3600
    );

    struct sigaction accion = {0};
    accion.sa_handler = manejar_senal;
    sigemptyset(&accion.sa_mask);

    sigaction(SIGINT, &accion, NULL);
    sigaction(SIGTERM, &accion, NULL);

    printf(
        "Simulador de degradacion controlada\n"
        "PID=%d PGID=%d objetivo=%ld lote=%ld intervalo_ms=%ld duracion_s=%ld\n",
        (int)getpid(),
        (int)getpgrp(),
        objetivo,
        lote,
        intervalo_ms,
        duracion_s
    );
    fflush(stdout);

    pid_t *hijos = calloc((size_t)objetivo, sizeof(pid_t));
    if (hijos == NULL) {
        perror("calloc");
        return 1;
    }

    long creados = 0;

    while (!terminar && creados < objetivo) {
        long por_crear = lote;

        if (creados + por_crear > objetivo)
            por_crear = objetivo - creados;

        for (long i = 0; i < por_crear && !terminar; ++i) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");
                terminar = 1;
                break;
            }

            if (pid == 0) {
                while (!terminar)
                    pause();

                _exit(0);
            }

            hijos[creados++] = pid;
        }

        printf("procesos_creados=%ld/%ld\n", creados, objetivo);
        fflush(stdout);

        struct timespec espera = {
            .tv_sec = intervalo_ms / 1000,
            .tv_nsec = (intervalo_ms % 1000) * 1000000L
        };

        nanosleep(&espera, NULL);
    }

    if (!terminar) {
        printf(
            "Objetivo alcanzado. Manteniendo carga durante %ld segundos.\n",
            duracion_s
        );
        fflush(stdout);

        for (long i = 0; i < duracion_s && !terminar; ++i)
            sleep(1);
    }

    printf("Finalizando procesos hijos...\n");
    fflush(stdout);

    for (long i = 0; i < creados; ++i) {
        if (hijos[i] > 0)
            kill(hijos[i], SIGTERM);
    }

    for (long i = 0; i < creados; ++i) {
        if (hijos[i] > 0)
            waitpid(hijos[i], NULL, 0);
    }

    free(hijos);

    printf("Simulador finalizado correctamente.\n");
    return 0;
}
