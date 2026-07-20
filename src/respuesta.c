#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "respuesta.h"

static char ruta_archivo_log[512] = "logs/eventos.log";
static unsigned int uid_laboratorio = (unsigned int)-1;
static int pgid_autorizado = -1;
static int segundos_gracia = 2;
static char ruta_pgid_autorizado[512];


/* ============================================================
 * Utilidades de logging
 * ============================================================ */

static void asegurar_directorio_log(const char *ruta_log) {
    char directorio[512];
    char *barra;

    snprintf(directorio, sizeof(directorio), "%s", ruta_log);

    barra = strrchr(directorio, '/');

    if (barra != NULL) {
        *barra = '\0';

        if (*directorio != '\0') {
            mkdir(directorio, 0755);
        }
    }
}


static void marca(char salida[26]) {
    time_t ahora = time(NULL);
    struct tm *tm_info = localtime(&ahora);

    strftime(
        salida,
        26,
        "%Y-%m-%d %H:%M:%S",
        tm_info
    );
}


static void escribir(FILE *archivo, const char *linea) {
    fputs(linea, stdout);

    if (archivo != NULL) {
        fputs(linea, archivo);
    }
}


/* ============================================================
 * Configuración pública
 * ============================================================ */

void configurar_logger(const char *ruta_log) {
    FILE *archivo;
    char tiempo[26];

    snprintf(
        ruta_archivo_log,
        sizeof(ruta_archivo_log),
        "%s",
        ruta_log
    );

    asegurar_directorio_log(ruta_archivo_log);

    archivo = fopen(ruta_archivo_log, "a");

    if (archivo != NULL) {
        marca(tiempo);

        fprintf(
            archivo,
            "%s INFO anti-bunny-virus iniciado\n",
            tiempo
        );

        fclose(archivo);
    }
}


void configurar_laboratorio(
    const char *usuario,
    int pgid,
    const char *ruta_pgid,
    int gracia
) {
    struct passwd *cuenta = getpwnam(usuario);

    uid_laboratorio =
        cuenta != NULL
            ? (unsigned int)cuenta->pw_uid
            : (unsigned int)-1;

    pgid_autorizado = pgid;

    if (ruta_pgid != NULL) {
        snprintf(
            ruta_pgid_autorizado,
            sizeof(ruta_pgid_autorizado),
            "%s",
            ruta_pgid
        );
    }

    segundos_gracia = gracia > 0 ? gracia : 2;
}


/* ============================================================
 * Autorización del PGID
 * ============================================================ */

static void refrescar_pgid_autorizado(void) {
    char linea[64];
    FILE *archivo;

    if (ruta_pgid_autorizado[0] == '\0') {
        return;
    }

    archivo = fopen(ruta_pgid_autorizado, "r");

    if (archivo == NULL) {
        return;
    }

    if (fgets(linea, sizeof(linea), archivo) != NULL) {
        int pgid = atoi(linea);

        if (pgid > 1) {
            pgid_autorizado = pgid;
        }
    }

    fclose(archivo);
}


/* ============================================================
 * Validación de identidad del proceso
 * ============================================================ */

static int leer_starttime_actual(
    int pid,
    unsigned long long *starttime
) {
    char ruta[64];
    char linea[4096];
    char *cierre;
    char *guardar = NULL;
    char *token;

    FILE *archivo;

    int indice = 0;

    snprintf(
        ruta,
        sizeof(ruta),
        "/proc/%d/stat",
        pid
    );

    archivo = fopen(ruta, "r");

    if (
        archivo == NULL ||
        fgets(linea, sizeof(linea), archivo) == NULL
    ) {
        if (archivo != NULL) {
            fclose(archivo);
        }

        return -1;
    }

    fclose(archivo);

    cierre = strrchr(linea, ')');

    if (cierre == NULL) {
        return -1;
    }

    token = strtok_r(
        cierre + 2,
        " ",
        &guardar
    );

    while (token != NULL) {

        if (indice == 19) {
            *starttime =
                strtoull(token, NULL, 10);

            return 0;
        }

        token = strtok_r(
            NULL,
            " ",
            &guardar
        );

        ++indice;
    }

    return -1;
}


static int identidad_actual_valida(
    const Alerta *a
) {
    char ruta[64];

    struct stat datos;

    unsigned long long starttime_actual;

    pid_t pgid_actual;

    if (
        a->starttime == 0 ||
        leer_starttime_actual(
            a->pid,
            &starttime_actual
        ) != 0
    ) {
        return 0;
    }

    snprintf(
        ruta,
        sizeof(ruta),
        "/proc/%d",
        a->pid
    );

    if (stat(ruta, &datos) != 0) {
        return 0;
    }

    pgid_actual = getpgid(a->pid);

    return
        starttime_actual == a->starttime &&
        pgid_actual == a->pgid &&
        (unsigned int)datos.st_uid == uid_laboratorio;
}


/* ============================================================
 * Política de autorización de contención
 * ============================================================ */

static int puede_actuar(
    const Alerta *a,
    const char **motivo
) {
    if (!a->accionable) {
        *motivo = "alerta_no_accionable";
        return 0;
    }

    if (strcmp(a->severidad, "critica") != 0) {
        *motivo = "alerta_no_critica";
        return 0;
    }

    if (a->pid <= 1) {
        *motivo = "pid_invalido";
        return 0;
    }

    if (a->pgid <= 1) {
        *motivo = "pgid_invalido";
        return 0;
    }

    if (a->pgid != pgid_autorizado) {
        *motivo = "pgid_no_autorizado";
        return 0;
    }

    if (a->uid != uid_laboratorio) {
        *motivo = "uid_no_autorizado";
        return 0;
    }

    if (!identidad_actual_valida(a)) {
        *motivo = "identidad_invalida";
        return 0;
    }

    *motivo = "autorizado";

    return 1;
}


/* ============================================================
 * Contención gradual del grupo autorizado
 *
 * Flujo:
 *
 * validar política
 *      ↓
 * SIGTERM al PGID
 *      ↓
 * periodo de gracia
 *      ↓
 * comprobar existencia y revalidar política
 *      ↓
 * SIGKILL solo si continúa activo y autorizado
 * ============================================================ */

static void terminar_grupo(
    FILE *archivo,
    const Alerta *a,
    const char *tiempo
) {
    char linea[768];

    const char *motivo = NULL;

    refrescar_pgid_autorizado();


    /* --------------------------------------------------------
     * Validación previa a cualquier señal
     * -------------------------------------------------------- */

    if (!puede_actuar(a, &motivo)) {

        snprintf(
            linea,
            sizeof(linea),
            "%s INFO accion=denegada "
            "pid=%d pgid=%d "
            "politica=contencion_laboratorio "
            "resultado=rechazado "
            "motivo=%s\n",
            tiempo,
            a->pid,
            a->pgid,
            motivo
        );

        escribir(archivo, linea);

        return;
    }


    snprintf(
        linea,
        sizeof(linea),
        "%s INFO accion=autorizada "
        "pid=%d pgid=%d "
        "politica=contencion_laboratorio "
        "resultado=permitido\n",
        tiempo,
        a->pid,
        a->pgid
    );

    escribir(archivo, linea);


    /* --------------------------------------------------------
     * Primera etapa: SIGTERM
     * -------------------------------------------------------- */

    if (kill(-a->pgid, SIGTERM) != 0) {

        int error_signal = errno;

        snprintf(
            linea,
            sizeof(linea),
            "%s ERROR accion=SIGTERM_GRUPO "
            "pgid=%d "
            "señal=SIGTERM "
            "resultado=error "
            "errno=%d\n",
            tiempo,
            a->pgid,
            error_signal
        );

        escribir(archivo, linea);

        return;
    }


    snprintf(
        linea,
        sizeof(linea),
        "%s INFO accion=SIGTERM_GRUPO "
        "pgid=%d "
        "señal=SIGTERM "
        "resultado=enviada "
        "gracia_s=%d\n",
        tiempo,
        a->pgid,
        segundos_gracia
    );

    escribir(archivo, linea);


    /* Periodo de gracia para terminación limpia. */

    sleep((unsigned int)segundos_gracia);


    /* --------------------------------------------------------
     * Segunda etapa: comprobar si el grupo continúa activo
     * -------------------------------------------------------- */

    errno = 0;

    int grupo_activo =
        kill(-a->pgid, 0) == 0 ||
        errno == EPERM;


    /*
     * Antes de escalar a SIGKILL se vuelve a validar la
     * identidad y autorización.
     */

    if (
        grupo_activo &&
        puede_actuar(a, &motivo)
    ) {

        /* ----------------------------------------------------
         * Último recurso: SIGKILL
         * ---------------------------------------------------- */

        if (kill(-a->pgid, SIGKILL) == 0) {

            snprintf(
                linea,
                sizeof(linea),
                "%s INFO accion=SIGKILL_GRUPO "
                "pgid=%d "
                "señal=SIGKILL "
                "resultado=enviada\n",
                tiempo,
                a->pgid
            );

        } else {

            int error_signal = errno;

            snprintf(
                linea,
                sizeof(linea),
                "%s ERROR accion=SIGKILL_GRUPO "
                "pgid=%d "
                "señal=SIGKILL "
                "resultado=error "
                "errno=%d\n",
                tiempo,
                a->pgid,
                error_signal
            );
        }

        escribir(archivo, linea);

    } else {

        snprintf(
            linea,
            sizeof(linea),
            "%s INFO "
            "accion=grupo_finalizado_o_identidad_cambiada "
            "pgid=%d "
            "resultado=sin_escalamiento\n",
            tiempo,
            a->pgid
        );

        escribir(archivo, linea);
    }
}


/* ============================================================
 * API principal de respuesta
 * ============================================================ */

void responder(
    Alerta *alertas,
    int n_alertas,
    const char *modo
) {
    FILE *archivo =
        fopen(ruta_archivo_log, "a");

    for (int i = 0; i < n_alertas; ++i) {

        char tiempo[26];
        char linea[1024];

        marca(tiempo);

        /*
         * Toda alerta queda registrada independientemente
         * del modo de respuesta.
         */

        snprintf(
            linea,
            sizeof(linea),
            "%s WARNING "
            "severidad=%s "
            "tipo=%s "
            "pid=%d "
            "pgid=%d "
            "starttime=%llu "
            "descripcion=\"%s\" "
            "evidencia=\"%s\" "
            "modo=%s\n",
            tiempo,
            alertas[i].severidad,
            alertas[i].tipo,
            alertas[i].pid,
            alertas[i].pgid,
            alertas[i].starttime,
            alertas[i].descripcion,
            alertas[i].evidencia,
            modo
        );

        escribir(archivo, linea);


        /*
         * El modo predeterminado "alerta" nunca envía señales.
         *
         * Solo "terminar_lab" puede solicitar contención,
         * y aun así terminar_grupo() aplica toda la política
         * de autorización antes de enviar señales.
         */

        if (strcmp(modo, "terminar_lab") == 0) {

            terminar_grupo(
                archivo,
                &alertas[i],
                tiempo
            );
        }
    }

    if (archivo != NULL) {
        fclose(archivo);
    }
}