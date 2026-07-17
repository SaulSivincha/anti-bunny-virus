#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    enum { LIMITE = 25 };
    pid_t hijos[LIMITE];
    int creados = 0;

    for (int i = 0; i < LIMITE; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            sleep(8);
            return 0;
        }
        if (pid < 0) break;
        hijos[creados++] = pid;
        struct timespec espera = { .tv_sec = 0, .tv_nsec = 50000000 };
        nanosleep(&espera, NULL);
    }
    printf("Simulador activo: %d procesos hijos durante 8 segundos\n", creados);
    for (int i = 0; i < creados; ++i) waitpid(hijos[i], NULL, 0);
    return 0;
}
