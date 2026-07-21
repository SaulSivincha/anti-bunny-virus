/*
 * Interfaz del motor de detección. Recibe las tres clases de muestras y genera
 * alertas con severidad, evidencia e identidad suficiente para responder.
 */
#ifndef MOTOR_DETECCION_H
#define MOTOR_DETECCION_H

#include "monitor_procesos.h"
#include "monitor_recursos.h"
#include "monitor_archivos.h"

// Evento producido cuando una regla supera su umbral y nivel de persistencia.
typedef struct {
    int pid;
    int pgid;
    unsigned long long starttime;
    unsigned int uid;
    int accionable;
    char severidad[16];
    char tipo[32];
    char descripcion[256];
    char evidencia[512];
} Alerta;

// Evalúa procesos, recursos y archivos; devuelve la cantidad de alertas creadas.
int detectar(
    ProcesoInfo* procesos, int n_procesos,
    RecursoInfo* recursos, int n_recursos,
    ArchivoInfo* archivos, int n_archivos,
    Alerta* alertas_out, int max_alertas,
    int max_hijos, int max_hijos_nuevos, int ciclos_persistencia,
    float max_memoria_mb, float max_delta_memoria_mb_s,
    float max_cpu, float max_crecimiento
);
// Limpia la persistencia interna; se usa principalmente para aislar pruebas.
void reiniciar_estado_deteccion(void);

#endif
