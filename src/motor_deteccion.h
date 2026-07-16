#ifndef MOTOR_DETECCION_H
#define MOTOR_DETECCION_H

#include "monitor_procesos.h"
#include "monitor_recursos.h"
#include "monitor_archivos.h"

typedef struct {
    char tipo[32];
    char descripcion[256];
    char evidencia[512];
} Alerta;

int detectar(
    ProcesoInfo* procesos, int n_procesos,
    RecursoInfo* recursos, int n_recursos,
    ArchivoInfo* archivos, int n_archivos,
    Alerta* alertas_out, int max_alertas,
    int max_hijos, float max_memoria_mb,
    float max_cpu, float max_crecimiento
);

#endif