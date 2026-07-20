#include <assert.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "respuesta.h"

/*
 * Lee el starttime real de un proceso desde /proc/<pid>/stat.
 * El campo starttime permite detectar reutilización de PID.
 */
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


/*
 * Inicializa una alerta crítica y accionable.
 */
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
    strcpy(alerta->descripcion, "prueba controlada de politica");
    strcpy(alerta->evidencia, "test_respuesta");
}


/*
 * Modo alerta:
 * responder() solo registra evidencia y nunca envía señales.
 */
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

    /* El hijo debe continuar vivo. */
    assert(kill(hijo, 0) == 0);

    /* Limpieza controlada del proceso creado por el test. */
    kill(hijo, SIGKILL);
    waitpid(hijo, NULL, 0);

    printf("OK: modo alerta no envia señales\n");
}


/*
 * Una alerta que no sea crítica nunca puede solicitar contención.
 */
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


/*
 * Una alerta marcada como no accionable debe rechazarse.
 */
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


/*
 * PID 1 queda explícitamente fuera del alcance de la contención.
 */
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


/*
 * Un PGID <= 1 nunca es válido para la política.
 */
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


/*
 * Prueba explícita de otro PGID.
 *
 * Se configura como autorizado un PGID diferente del indicado
 * en la alerta. La política debe detenerse exactamente en
 * pgid_no_autorizado.
 */
static void test_otro_pgid_rechazado(const char *usuario) {
    Alerta alerta;
    pid_t pgid_real = getpgrp();

    configurar_laboratorio(
        usuario,
        pgid_real + 10000,
        NULL,
        1
    );

    preparar_alerta(
        &alerta,
        getpid(),
        pgid_real,
        (unsigned int)getuid(),
        obtener_starttime(getpid())
    );

    responder(&alerta, 1, "terminar_lab");

    printf("OK: PGID no autorizado rechazado\n");
}


/*
 * Prueba explícita de UID incorrecto.
 *
 * Aquí el PGID sí coincide con el autorizado para permitir
 * que la validación alcance específicamente la comprobación UID.
 */
static void test_uid_no_autorizado(const char *usuario) {
    Alerta alerta;
    pid_t pgid_real = getpgrp();

    configurar_laboratorio(
        usuario,
        pgid_real,
        NULL,
        1
    );

    preparar_alerta(
        &alerta,
        getpid(),
        pgid_real,
        (unsigned int)getuid() + 1U,
        obtener_starttime(getpid())
    );

    responder(&alerta, 1, "terminar_lab");

    printf("OK: UID no autorizado rechazado\n");
}


/*
 * Prueba explícita de identidad/starttime incorrecto.
 *
 * PGID y UID son válidos. Se altera únicamente starttime,
 * simulando una identidad de proceso obsoleta o un PID reutilizado.
 */
static void test_starttime_incorrecto(const char *usuario) {
    Alerta alerta;
    pid_t pgid_real = getpgrp();
    unsigned long long starttime_real =
        obtener_starttime(getpid());

    configurar_laboratorio(
        usuario,
        pgid_real,
        NULL,
        1
    );

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


int main(void) {
    struct passwd *cuenta = getpwuid(getuid());

    assert(cuenta != NULL);

    configurar_logger("logs/test_respuesta.log");

    test_modo_alerta_no_termina_proceso();
    test_alerta_no_critica_no_actua();
    test_alerta_no_accionable();
    test_pid_1_rechazado();
    test_pgid_invalido();

    test_otro_pgid_rechazado(cuenta->pw_name);
    test_uid_no_autorizado(cuenta->pw_name);
    test_starttime_incorrecto(cuenta->pw_name);

    printf("test_respuesta: OK\n");

    return 0;
}