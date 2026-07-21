#include <stdio.h>
#include <string.h>

#include "motor_deteccion.h"

/* Estado interno de persistencia por tipo de señal y proceso o archivo. */
#define MAX_ESTADOS_ALERTA 8192
typedef struct {
    char tipo[32];
    int pid;
    unsigned long long starttime;
    int ciclos;
    int nivel_emitido;
} EstadoAlerta;
static EstadoAlerta estados[MAX_ESTADOS_ALERTA];
static int total_estados;

/* Función para reiniciar el historial de persistencia (usada en pruebas unitarias). */
void reiniciar_estado_deteccion(void) {
    memset(estados, 0, sizeof(estados));
    total_estados = 0;
}

/* Función para contar ciclos consecutivos en los que una condición sigue activa. */
static EstadoAlerta *actualizar_persistencia(const char *tipo, int pid,
                                             unsigned long long starttime, int activo) {
    for (int i = 0; i < total_estados; ++i) {
        if (estados[i].pid == pid && estados[i].starttime == starttime &&
            strcmp(estados[i].tipo, tipo) == 0) {
            estados[i].ciclos = activo ? estados[i].ciclos + 1 : 0;
            if (!activo) estados[i].nivel_emitido = 0;
            return &estados[i];
        }
    }
    if (total_estados < MAX_ESTADOS_ALERTA) {
        snprintf(estados[total_estados].tipo, sizeof(estados[total_estados].tipo), "%s", tipo);
        estados[total_estados].pid = pid;
        estados[total_estados].starttime = starttime;
        estados[total_estados].ciclos = activo ? 1 : 0;
        return &estados[total_estados++];
    }
    return NULL;
}

/* Función para añadir una alerta estructurada al buffer de salida del ciclo. */
static void agregar(Alerta *salida, int *total, int max, int pid, int pgid,
                    unsigned long long starttime, unsigned int uid, int accionable,
                    const char *severidad, const char *tipo,
                    const char *descripcion, const char *evidencia) {
    if (*total >= max) return;
    Alerta *a = &salida[(*total)++];
    memset(a, 0, sizeof(*a));
    a->pid = pid; a->pgid = pgid; a->starttime = starttime;
    a->uid = uid; a->accionable = accionable;
    snprintf(a->severidad, sizeof(a->severidad), "%s", severidad);
    snprintf(a->tipo, sizeof(a->tipo), "%s", tipo);
    snprintf(a->descripcion, sizeof(a->descripcion), "%s", descripcion);
    snprintf(a->evidencia, sizeof(a->evidencia), "%s", evidencia);
}

/* Función para enlazar métricas de archivo con el consumo del PID escritor. */
static const RecursoInfo *recurso_de_pid(const RecursoInfo *recursos, int n, int pid) {
    for (int i = 0; i < n; ++i) if (recursos[i].pid == pid) return &recursos[i];
    return NULL;
}

/*
 * Función principal de detección: evalúa procesos, recursos y archivos,
 * aplica persistencia y correlación, y genera alertas sospechosas o críticas.
 */
int detectar(ProcesoInfo *procesos, int n_procesos, RecursoInfo *recursos, int n_recursos,
             ArchivoInfo *archivos, int n_archivos, Alerta *alertas_out, int max_alertas,
             int max_hijos, int max_hijos_nuevos, int ciclos_persistencia,
             float max_memoria_mb, float max_delta_memoria_mb_s, float max_cpu,
             float max_crecimiento) {
    int total = 0;

    /* Reglas de proliferación de procesos. */
    for (int i = 0; i < n_procesos; ++i) {
        int activo = procesos[i].hijos >= max_hijos || procesos[i].hijos >= max_hijos_nuevos ||
                     procesos[i].hijos_nuevos >= max_hijos_nuevos;
        EstadoAlerta *estado = actualizar_persistencia(
            "procesos", procesos[i].pid, procesos[i].starttime, activo);
        int ciclos = estado != NULL ? estado->ciclos : 0;
        int critica = ciclos >= ciclos_persistencia || procesos[i].hijos >= max_hijos * 2;
        int nivel = critica ? 2 : 1;
        if (activo && estado != NULL && estado->nivel_emitido < nivel) {
            char evidencia[512];
            snprintf(evidencia, sizeof(evidencia), "pid=%d ppid=%d pgid=%d usuario=%s hijos=%d nuevos=%d tasa_hijos_s=%.2f ciclos=%d",
                     procesos[i].pid, procesos[i].ppid, procesos[i].pgid, procesos[i].usuario,
                     procesos[i].hijos, procesos[i].hijos_nuevos, procesos[i].hijos_nuevos_s, ciclos);
            agregar(alertas_out, &total, max_alertas, procesos[i].pid, procesos[i].pgid,
                    procesos[i].starttime, procesos[i].uid, critica,
                    critica ? "critica" : "sospechosa", "procesos",
                    critica ? "Proliferacion de procesos persistente" : "Proliferacion de procesos observada", evidencia);
            estado->nivel_emitido = nivel;
        }
    }
    /* Reglas de memoria, CPU y crecimiento de RSS sostenido. */
    for (int i = 0; i < n_recursos; ++i) {
        int activo = recursos[i].memoria_mb > max_memoria_mb || recursos[i].cpu_porcentaje > max_cpu ||
                     recursos[i].delta_memoria_mb_s > max_delta_memoria_mb_s;
        EstadoAlerta *estado = actualizar_persistencia(
            "recursos", recursos[i].pid, recursos[i].starttime, activo);
        int ciclos = estado != NULL ? estado->ciclos : 0;
        if (activo && estado != NULL && ciclos >= ciclos_persistencia && estado->nivel_emitido < 1) {
            char evidencia[512];
            snprintf(evidencia, sizeof(evidencia), "pid=%d nombre=%s memoria_mb=%.2f delta_rss_mb_s=%.2f cpu=%.2f ciclos=%d",
                     recursos[i].pid, recursos[i].nombre, recursos[i].memoria_mb,
                     recursos[i].delta_memoria_mb_s, recursos[i].cpu_porcentaje, ciclos);
            agregar(alertas_out, &total, max_alertas, recursos[i].pid, -1,
                    recursos[i].starttime, 0, 0, "sospechosa",
                    "recursos", "Consumo de recursos persistente", evidencia);
            estado->nivel_emitido = 1;
        }
    }
    /* Reglas de ficheros que crecen rápido, con correlación opcional al escritor. */
    for (int i = 0; i < n_archivos; ++i) {
        int activo = archivos[i].crecimiento_mb_s > max_crecimiento;
        int clave = archivos[i].pid_escritor > 0 ? archivos[i].pid_escritor : -(i + 1);
        EstadoAlerta *estado = actualizar_persistencia("archivos", clave, 0, activo);
        int ciclos = estado != NULL ? estado->ciclos : 0;
        const RecursoInfo *r = recurso_de_pid(recursos, n_recursos, archivos[i].pid_escritor);
        int correlacion = r != NULL && (r->cpu_porcentaje > max_cpu || r->delta_memoria_mb_s > max_delta_memoria_mb_s);
        int nivel = correlacion ? 2 : 1;
        if (activo && estado != NULL && ciclos >= ciclos_persistencia && estado->nivel_emitido < nivel) {
            char evidencia[512];
            snprintf(evidencia, sizeof(evidencia), "ruta=%s bytes=%lld crecimiento_mb_s=%.2f pid_escritor=%d pgid=%d atribuido=%d ciclos=%d",
                     archivos[i].ruta, archivos[i].tamano_bytes, archivos[i].crecimiento_mb_s,
                     archivos[i].pid_escritor, archivos[i].pgid_escritor, archivos[i].atribuido, ciclos);
            agregar(alertas_out, &total, max_alertas, archivos[i].pid_escritor,
                    archivos[i].pgid_escritor, 0, 0, 0,
                    correlacion ? "critica" : "sospechosa", "archivos",
                    correlacion ? "Archivo con crecimiento correlacionado" : "Archivo con crecimiento persistente", evidencia);
            estado->nivel_emitido = nivel;
        }
    }
    return total;
}
