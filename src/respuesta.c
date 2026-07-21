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
static int segundos_gracia = 2;

/* Función para crear la carpeta padre del archivo de log si no existe. */
static void asegurar_directorio_log(const char *ruta_log) {
    char directorio[512];
    char *barra;
    if (ruta_log == NULL) return;
    snprintf(directorio, sizeof(directorio), "%s", ruta_log);
    barra = strrchr(directorio, '/');
    if (barra != NULL) {
        *barra = '\0';
        if (*directorio != '\0') {
            mkdir(directorio, 0755);
        }
    }
}

/* Función para formatear la marca de tiempo humana de cada evento. */
static void marca(char salida[26]) {
    time_t ahora = time(NULL);
    struct tm *tm_info = localtime(&ahora);
    if (tm_info != NULL) {
        strftime(salida, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(salida, 26, "1970-01-01 00:00:00");
    }
}

/* Función para duplicar cada línea de log en consola y en disco. */
static void escribir(FILE *archivo, const char *linea) {
    if (linea == NULL) return;
    fputs(linea, stdout);
    if (archivo != NULL) {
        fputs(linea, archivo);
    }
}

/* Función para fijar la ruta del log y registrar el arranque del monitor. */
void configurar_logger(const char *ruta_log) {
    FILE *archivo;
    char tiempo[26];
    if (ruta_log != NULL && ruta_log[0] != '\0') {
        snprintf(ruta_archivo_log, sizeof(ruta_archivo_log), "%s", ruta_log);
    }
    asegurar_directorio_log(ruta_archivo_log);
    archivo = fopen(ruta_archivo_log, "a");
    if (archivo != NULL) {
        marca(tiempo);
        fprintf(archivo, "%s INFO anti-bunny-virus iniciado\n", tiempo);
        fclose(archivo);
    }
}

/* Carga la política de laboratorio garantizando la resolución del UID del usuario */
void configurar_laboratorio(const char *usuario, int pgid, const char *ruta_pgid, int gracia) {
    (void)pgid;      // Ignorado para no depender de un PGID fijo
    (void)ruta_pgid;  // Ignorado para no depender de archivo PGID
    
    if (usuario != NULL) {
        struct passwd *cuenta = getpwnam(usuario);
        if (cuenta != NULL) {
            uid_laboratorio = (unsigned int)cuenta->pw_uid;
        } else {
            uid_laboratorio = (unsigned int)-1;
        }
    } else {
        uid_laboratorio = (unsigned int)-1;
    }
    segundos_gracia = gracia > 0 ? gracia : 2;
}

/* Función para leer el starttime actual desde /proc y detectar reutilización de PID. */
static int leer_starttime_actual(int pid, unsigned long long *starttime) {
    char ruta[64];
    char linea[4096];
    char *cierre;
    char *guardar = NULL;
    char *token;
    FILE *archivo;
    int indice = 0;

    if (pid <= 1 || starttime == NULL) return -1;

    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL) return -1;

    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        fclose(archivo);
        return -1;
    }
    fclose(archivo);

    cierre = strrchr(linea, ')');
    if (cierre == NULL) return -1;

    token = strtok_r(cierre + 2, " ", &guardar);
    while (token != NULL) {
        if (indice == 19) {
            *starttime = strtoull(token, NULL, 10);
            return 0;
        }
        token = strtok_r(NULL, " ", &guardar);
        ++indice;
    }
    return -1;
}

/* Comprueba que el proceso sigue activo y pertenece al usuario del laboratorio */
static int identidad_actual_valida(const Alerta *a) {
    char ruta[64];
    struct stat datos;
    unsigned long long starttime_actual = 0;

    if (a == NULL || a->pid <= 1) return 0;

    if (a->starttime != 0) {
        if (leer_starttime_actual(a->pid, &starttime_actual) != 0) {
            return 0;
        }
        if (starttime_actual != a->starttime) {
            return 0;
        }
    }

    snprintf(ruta, sizeof(ruta), "/proc/%d", a->pid);
    if (stat(ruta, &datos) != 0) {
        return 0;
    }

    return (uid_laboratorio == (unsigned int)-1 || datos.st_uid == uid_laboratorio);
}

/* Decisión dinámica de contención */
static int puede_actuar(const Alerta *a, const char **motivo) {
    if (a == NULL || motivo == NULL) return 0;

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
    if (uid_laboratorio != (unsigned int)-1 && a->uid != uid_laboratorio) {
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

/* Función para terminar dinámicamente el grupo detectado */
static void terminar_grupo(FILE *archivo, const Alerta *a, const char *tiempo) {
    char linea[768];
    const char *motivo = NULL;

    if (!puede_actuar(a, &motivo)) {
        snprintf(linea, sizeof(linea),
                 "%s INFO accion=denegada pid=%d pgid=%d politica=contencion_laboratorio "
                 "resultado=rechazado motivo=%s\n",
                 tiempo, a->pid, a->pgid, motivo);
        escribir(archivo, linea);
        return;
    }

    snprintf(linea, sizeof(linea),
             "%s INFO accion=autorizada pid=%d pgid=%d politica=contencion_laboratorio "
             "resultado=permitido\n",
             tiempo, a->pid, a->pgid);
    escribir(archivo, linea);

    // Envía SIGTERM al grupo completo de procesos (usa -pgid)
    if (kill(-a->pgid, SIGTERM) != 0) {
        int error_signal = errno;
        snprintf(linea, sizeof(linea),
                 "%s ERROR accion=SIGTERM_GRUPO pgid=%d senal=SIGTERM resultado=error errno=%d\n",
                 tiempo, a->pgid, error_signal);
        escribir(archivo, linea);
        return;
    }

    snprintf(linea, sizeof(linea),
             "%s INFO accion=SIGTERM_GRUPO pgid=%d senal=SIGTERM resultado=enviada gracia_s=%d\n",
             tiempo, a->pgid, segundos_gracia);
    escribir(archivo, linea);

    sleep((unsigned int)segundos_gracia);

    errno = 0;
    int grupo_activo = kill(-a->pgid, 0) == 0 || errno == EPERM;
    if (grupo_activo && puede_actuar(a, &motivo)) {
        if (kill(-a->pgid, SIGKILL) == 0) {
            snprintf(linea, sizeof(linea),
                     "%s INFO accion=SIGKILL_GRUPO pgid=%d senal=SIGKILL resultado=enviada\n",
                     tiempo, a->pgid);
        } else {
            int error_signal = errno;
            snprintf(linea, sizeof(linea),
                     "%s ERROR accion=SIGKILL_GRUPO pgid=%d senal=SIGKILL resultado=error errno=%d\n",
                     tiempo, a->pgid, error_signal);
        }
        escribir(archivo, linea);
    } else {
        snprintf(linea, sizeof(linea),
                 "%s INFO accion=grupo_finalizado_o_identidad_cambiada pgid=%d resultado=sin_escalamiento\n",
                 tiempo, a->pgid);
        escribir(archivo, linea);
    }
}

/* Función principal de respuesta */
void responder(Alerta *alertas, int n_alertas, const char *modo) {
    FILE *archivo;
    if (alertas == NULL || n_alertas <= 0) return;

    archivo = fopen(ruta_archivo_log, "a");
    for (int i = 0; i < n_alertas; ++i) {
        char tiempo[26];
        char linea[1024];
        marca(tiempo);
        snprintf(linea, sizeof(linea),
                 "%s WARNING severidad=%s tipo=%s pid=%d pgid=%d starttime=%llu "
                 "descripcion=\"%s\" evidencia=\"%s\" modo=%s\n",
                 tiempo, alertas[i].severidad, alertas[i].tipo,
                 alertas[i].pid, alertas[i].pgid, alertas[i].starttime,
                 alertas[i].descripcion, alertas[i].evidencia,
                 modo != NULL ? modo : "alerta");
        escribir(archivo, linea);

        if (modo != NULL && strcmp(modo, "terminar_lab") == 0) {
            terminar_grupo(archivo, &alertas[i], tiempo);
        }
    }
    if (archivo != NULL) {
        fclose(archivo);
    }
}