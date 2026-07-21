#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "respuesta.h"

static unsigned long long obtener_starttime(pid_t pid) {
    char ruta[64];
    char linea[4096];
    char *cierre;
    char *guardar = NULL;
    char *token;
    FILE *archivo;
    int indice = 0;

    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);

    archivo = fopen(ruta, "r");
    assert(archivo != NULL);

    assert(fgets(linea, sizeof(linea), archivo) != NULL);
    fclose(archivo);

    cierre = strrchr(linea, ')');
    assert(cierre != NULL);

    token = strtok_r(cierre + 2, " ", &guardar);

    while (token != NULL) {
        if (indice == 19) {
            return strtoull(token, NULL, 10);
        }

        token = strtok_r(NULL, " ", &guardar);
        ++indice;
    }

    assert(0 && "No se pudo obtener starttime");
    return 0;
}

static void preparar_alerta(
    Alerta *alerta,
    pid_t pid,
    pid_t pgid,
    unsigned int uid,
    unsigned long long starttime
) {
    memset(alerta, 0, sizeof(*alerta));

    alerta->pid = pid;
    alerta->pgid = pgid;
    alerta->uid = uid;
    alerta->starttime = starttime;
    alerta->accionable = 1;

    strcpy(alerta->severidad, "critica");
    strcpy(alerta->tipo, "procesos");
    strcpy(alerta->descripcion, "prueba de contencion por alerta");
    strcpy(alerta->evidencia, "test_respuesta");
}

static void test_modo_alerta_no_termina_proceso(void) {
    pid_t hijo = fork();

    assert(hijo >= 0);

    if (hijo == 0) {
        while (1) {
            pause();
        }
    }

    Alerta alerta;

    preparar_alerta(
        &alerta,
        hijo,
        getpgid(hijo),
        (unsigned int)getuid(),
        0
    );

    responder(&alerta, 1, "alerta");

    assert(kill(hijo, 0) == 0);

    kill(hijo, SIGKILL);
    waitpid(hijo, NULL, 0);

    printf("OK: modo alerta no envia señales\n");
}

static void test_alerta_no_critica_no_actua(void) {
    Alerta alerta;

    preparar_alerta(
        &alerta,
        getpid(),
        getpgrp(),
        (unsigned int)getuid(),
        obtener_starttime(getpid())
    );

    strcpy(alerta.severidad, "sospechosa");

    responder(&alerta, 1, "terminar_lab");

    printf("OK: alerta no critica rechazada\n");
}

static void test_alerta_no_accionable(void) {
    Alerta alerta;

    preparar_alerta(
        &alerta,
        getpid(),
        getpgrp(),
        (unsigned int)getuid(),
        obtener_starttime(getpid())
    );

    alerta.accionable = 0;

    responder(&alerta, 1, "terminar_lab");

    printf("OK: alerta no accionable rechazada\n");
}

static void test_tipo_no_procesos_no_actua(void) {
    Alerta alerta;

    preparar_alerta(
        &alerta,
        getpid(),
        getpgrp(),
        (unsigned int)getuid(),
        obtener_starttime(getpid())
    );

    strcpy(alerta.tipo, "recursos");

    responder(&alerta, 1, "terminar_lab");

    printf("OK: tipo no procesos rechazado\n");
}

static void test_pid_1_rechazado(void) {
    Alerta alerta;

    preparar_alerta(
        &alerta,
        1,
        getpgrp(),
        (unsigned int)getuid(),
        1
    );

    responder(&alerta, 1, "terminar_lab");

    printf("OK: PID 1 rechazado\n");
}

static void test_pgid_invalido(void) {
    Alerta alerta;

    preparar_alerta(
        &alerta,
        getpid(),
        -1,
        (unsigned int)getuid(),
        obtener_starttime(getpid())
    );

    responder(&alerta, 1, "terminar_lab");

    printf("OK: PGID invalido rechazado\n");
}

static void test_starttime_incorrecto(void) {
    Alerta alerta;
    pid_t pgid_real = getpgrp();
    unsigned long long starttime_real = obtener_starttime(getpid());

    preparar_alerta(
        &alerta,
        getpid(),
        pgid_real,
        (unsigned int)getuid(),
        starttime_real + 1
    );

    responder(&alerta, 1, "terminar_lab");

    printf("OK: starttime incorrecto rechazado\n");
}

static void test_contencion_termina_grupo_de_la_alerta(void) {
    pid_t hijo = fork();

    assert(hijo >= 0);

    if (hijo == 0) {
        setpgid(0, 0);
        while (1) {
            pause();
        }
    }

    struct timespec espera = { .tv_sec = 0, .tv_nsec = 100000000L };
    nanosleep(&espera, NULL);

    pid_t pgid_hijo = getpgid(hijo);
    assert(pgid_hijo > 1);

    Alerta alerta;

    preparar_alerta(
        &alerta,
        hijo,
        pgid_hijo,
        (unsigned int)getuid(),
        obtener_starttime(hijo)
    );

    configurar_laboratorio(NULL, -1, NULL, 1);
    responder(&alerta, 1, "terminar_lab");

    int status = 0;
    pid_t fin = waitpid(hijo, &status, 0);
    assert(fin == hijo);
    assert(WIFSIGNALED(status));

    printf("OK: contencion termina el PGID de la alerta\n");
}

int main(void) {
    configurar_logger("logs/test_respuesta.log");

    test_modo_alerta_no_termina_proceso();
    test_alerta_no_critica_no_actua();
    test_alerta_no_accionable();
    test_tipo_no_procesos_no_actua();
    test_pid_1_rechazado();
    test_pgid_invalido();
    test_starttime_incorrecto();
    test_contencion_termina_grupo_de_la_alerta();

    printf("test_respuesta: OK\n");

    return 0;
}
