#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t terminar = 0;

static void manejar_senal(int signo) {
    (void)signo;
    terminar = 1;
}

static long leer_entero(
    const char *nombre,
    long defecto,
    long minimo,
    long maximo
) {
    const char *valor = getenv(nombre);
    char *fin = NULL;
    long numero;

    if (valor == NULL || *valor == '\0')
        return defecto;

    errno = 0;
    numero = strtol(valor, &fin, 10);

    if (errno != 0 ||
        fin == valor ||
        *fin != '\0' ||
        numero < minimo ||
        numero > maximo) {

        fprintf(
            stderr,
            "Valor invalido para %s=%s; usando %ld\n",
            nombre,
            valor,
            defecto
        );

        return defecto;
    }

    return numero;
}

static void proceso_pasivo(void) {
    while (!terminar)
        pause();

    _exit(0);
}

static void worker_cpu(void) {
    volatile unsigned long long valor = 1;

    while (!terminar) {
        for (unsigned long i = 0; i < 1000000UL; ++i)
            valor = valor * 1664525ULL + 1013904223ULL;
    }

    (void)valor;
    _exit(0);
}

static unsigned char *reservar_memoria(long memoria_mb) {
    if (memoria_mb <= 0)
        return NULL;

    size_t bytes = (size_t)memoria_mb * 1024U * 1024U;

    unsigned char *memoria = malloc(bytes);

    if (memoria == NULL) {
        perror("malloc memoria de carga");
        return NULL;
    }

    /*
     * Tocamos cada pagina para forzar asignacion fisica real
     * en lugar de depender solo de memoria virtual reservada.
     */
    long pagina = sysconf(_SC_PAGESIZE);

    if (pagina <= 0)
        pagina = 4096;

    for (size_t i = 0; i < bytes; i += (size_t)pagina)
        memoria[i] = (unsigned char)(i & 0xFF);

    if (bytes > 0)
        memoria[bytes - 1] = 1;

    return memoria;
}

static void dormir_ms(long milisegundos) {
    struct timespec espera = {
        .tv_sec = milisegundos / 1000,
        .tv_nsec = (milisegundos % 1000) * 1000000L
    };

    while (!terminar && nanosleep(&espera, &espera) != 0) {
        if (errno != EINTR)
            break;
    }
}

int main(void) {
    long objetivo = leer_entero(
        "DEGRADACION_PROCESOS", 128, 0, 5000
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

    long memoria_mb = leer_entero(
        "DEGRADACION_MEMORIA_MB", 0, 0, 1024
    );

    long cpu_workers = leer_entero(
        "DEGRADACION_CPU_WORKERS", 0, 0, 16
    );

    struct sigaction accion = {0};

    accion.sa_handler = manejar_senal;
    sigemptyset(&accion.sa_mask);

    sigaction(SIGINT, &accion, NULL);
    sigaction(SIGTERM, &accion, NULL);

    printf(
        "Simulador de degradacion controlada\n"
        "PID=%d PGID=%d procesos=%ld lote=%ld "
        "intervalo_ms=%ld duracion_s=%ld "
        "memoria_mb=%ld cpu_workers=%ld\n",
        (int)getpid(),
        (int)getpgrp(),
        objetivo,
        lote,
        intervalo_ms,
        duracion_s,
        memoria_mb,
        cpu_workers
    );

    fflush(stdout);

    /*
     * La memoria se reserva en el proceso padre despues de crear
     * los hijos pasivos. De esta forma evitamos multiplicar
     * accidentalmente la carga de RAM entre todos los procesos.
     */
    pid_t *hijos = NULL;

    if (objetivo > 0) {
        hijos = calloc((size_t)objetivo, sizeof(pid_t));

        if (hijos == NULL) {
            perror("calloc hijos");
            return 1;
        }
    }

    pid_t *workers = NULL;

    if (cpu_workers > 0) {
        workers = calloc((size_t)cpu_workers, sizeof(pid_t));

        if (workers == NULL) {
            perror("calloc workers");
            free(hijos);
            return 1;
        }
    }

    long creados = 0;

    while (!terminar && creados < objetivo) {
        long por_crear = lote;

        if (creados + por_crear > objetivo)
            por_crear = objetivo - creados;

        for (long i = 0; i < por_crear && !terminar; ++i) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork proceso pasivo");
                terminar = 1;
                break;
            }

            if (pid == 0)
                proceso_pasivo();

            hijos[creados++] = pid;
        }

        printf(
            "fase=procesos procesos_creados=%ld/%ld\n",
            creados,
            objetivo
        );

        fflush(stdout);

        dormir_ms(intervalo_ms);
    }

    unsigned char *memoria = NULL;

    if (!terminar && memoria_mb > 0) {
        printf(
            "fase=memoria reservando=%ld_MB\n",
            memoria_mb
        );

        fflush(stdout);

        memoria = reservar_memoria(memoria_mb);

        if (memoria == NULL) {
            fprintf(
                stderr,
                "No fue posible reservar la memoria solicitada.\n"
            );

            terminar = 1;
        } else {
            printf(
                "fase=memoria reservada=%ld_MB\n",
                memoria_mb
            );

            fflush(stdout);
        }
    }

    long workers_creados = 0;

    if (!terminar && cpu_workers > 0) {
        for (long i = 0; i < cpu_workers; ++i) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork worker CPU");
                terminar = 1;
                break;
            }

            if (pid == 0)
                worker_cpu();

            workers[workers_creados++] = pid;
        }

        printf(
            "fase=cpu workers_creados=%ld/%ld\n",
            workers_creados,
            cpu_workers
        );

        fflush(stdout);
    }

    if (!terminar) {
        printf(
            "fase=carga_estable duracion_s=%ld\n",
            duracion_s
        );

        fflush(stdout);

        for (long i = 0; i < duracion_s && !terminar; ++i)
            sleep(1);
    }

    printf("fase=limpieza iniciando\n");
    fflush(stdout);

    for (long i = 0; i < workers_creados; ++i) {
        if (workers[i] > 0)
            kill(workers[i], SIGTERM);
    }

    for (long i = 0; i < creados; ++i) {
        if (hijos[i] > 0)
            kill(hijos[i], SIGTERM);
    }

    for (long i = 0; i < workers_creados; ++i) {
        if (workers[i] > 0)
            waitpid(workers[i], NULL, 0);
    }

    for (long i = 0; i < creados; ++i) {
        if (hijos[i] > 0)
            waitpid(hijos[i], NULL, 0);
    }

    free(memoria);
    free(workers);
    free(hijos);

    printf(
        "fase=finalizado procesos=%ld "
        "workers_cpu=%ld memoria_mb=%ld\n",
        creados,
        workers_creados,
        memoria_mb
    );

    fflush(stdout);

    return 0;
}
