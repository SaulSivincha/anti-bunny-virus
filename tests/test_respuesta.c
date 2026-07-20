#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "respuesta.h"

static void test_modo_alerta_no_termina_proceso(void) {
    pid_t hijo = fork();

    assert(hijo >= 0);

    if (hijo == 0) {
        /* Proceso controlado que permanece vivo durante la prueba. */
        while (1) {
            pause();
        }
    }

    Alerta alerta;
    memset(&alerta, 0, sizeof(alerta));

    alerta.pid = hijo;
    alerta.pgid = getpgid(hijo);
    alerta.accionable = 1;

    strcpy(alerta.severidad, "critica");
    strcpy(alerta.tipo, "procesos");
    strcpy(alerta.descripcion, "prueba controlada");
    strcpy(alerta.evidencia, "test modo alerta");

    /*
     * En modo "alerta", responder() solo debe registrar evidencia.
     * Nunca debe enviar SIGTERM ni SIGKILL.
     */
    responder(&alerta, 1, "alerta");

    /* kill(pid, 0) comprueba existencia sin enviar una señal real. */
    assert(kill(hijo, 0) == 0);

    /* Limpieza exclusiva del proceso creado por este test. */
    kill(hijo, SIGKILL);
    waitpid(hijo, NULL, 0);

    printf("OK: modo alerta no envia señales\n");
}

static void test_alerta_no_critica_no_actua(void) {
    pid_t hijo = fork();

    assert(hijo >= 0);

    if (hijo == 0) {
        while (1) {
            pause();
        }
    }

    Alerta alerta;
    memset(&alerta, 0, sizeof(alerta));

    alerta.pid = hijo;
    alerta.pgid = getpgid(hijo);
    alerta.accionable = 1;

    strcpy(alerta.severidad, "sospechosa");
    strcpy(alerta.tipo, "procesos");
    strcpy(alerta.descripcion, "alerta no critica");
    strcpy(alerta.evidencia, "test politica");

    /*
     * Aunque se solicite terminar_lab, una alerta que no sea crítica
     * debe ser rechazada por la política de contención.
     */
    responder(&alerta, 1, "terminar_lab");

    assert(kill(hijo, 0) == 0);

    kill(hijo, SIGKILL);
    waitpid(hijo, NULL, 0);

    printf("OK: alerta no critica no ejecuta contencion\n");
}

static void test_politica_rechaza_alertas_invalidas(void) {
    Alerta alerta;
    memset(&alerta, 0, sizeof(alerta));

    strcpy(alerta.severidad, "critica");
    strcpy(alerta.tipo, "procesos");
    strcpy(alerta.descripcion, "prueba de rechazo");
    strcpy(alerta.evidencia, "test politica");

    /*
     * Caso 1: alerta no accionable.
     */
    alerta.pid = getpid();
    alerta.pgid = getpgrp();
    alerta.accionable = 0;

    responder(&alerta, 1, "terminar_lab");

    /* Si llegamos aquí, la política rechazó la solicitud sin afectarnos. */
    printf("OK: alerta no accionable rechazada\n");

    /*
     * Caso 2: PID inválido.
     */
    alerta.pid = 1;
    alerta.pgid = getpgrp();
    alerta.accionable = 1;

    responder(&alerta, 1, "terminar_lab");

    printf("OK: PID 1 rechazado\n");

    /*
     * Caso 3: PGID inválido.
     */
    alerta.pid = getpid();
    alerta.pgid = -1;
    alerta.accionable = 1;

    responder(&alerta, 1, "terminar_lab");

    printf("OK: PGID invalido rechazado\n");

    /*
     * Caso 4: UID no autorizado.
     */
    alerta.pid = getpid();
    alerta.pgid = getpgrp();
    alerta.uid = (unsigned int)-1;
    alerta.accionable = 1;

    responder(&alerta, 1, "terminar_lab");

    printf("OK: UID no autorizado rechazado\n");

    /*
     * Caso 5: identidad/starttime inválido.
     */
    alerta.pid = getpid();
    alerta.pgid = getpgrp();
    alerta.uid = getuid();
    alerta.starttime = 1;
    alerta.accionable = 1;

    responder(&alerta, 1, "terminar_lab");

    printf("OK: starttime incorrecto rechazado\n");
}

int main(void) {
    configurar_logger("logs/test_respuesta.log");

    test_modo_alerta_no_termina_proceso();
    test_alerta_no_critica_no_actua();
    test_politica_rechaza_alertas_invalidas();

    printf("test_respuesta: OK\n");

    return 0;
}