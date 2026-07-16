#include <stdio.h>
#include <string.h>
#include "motor_deteccion.h"

int detectar(
    ProcesoInfo* procesos, int n_procesos,
    RecursoInfo* recursos, int n_recursos,
    ArchivoInfo* archivos, int n_archivos,
    Alerta* alertas_out, int max_alertas,
    int max_hijos, float max_memoria_mb,
    float max_cpu, float max_crecimiento
) {
    int total_alertas = 0;

    // Detección de procesos (Fork Bomb)
    for (int i = 0; i < n_procesos; i++) {
        if (procesos[i].hijos > max_hijos) {
            if (total_alertas < max_alertas) {
                strcpy(alertas_out[total_alertas].tipo, "procesos");
                strcpy(alertas_out[total_alertas].descripcion, "Proceso con demasiados hijos");
                snprintf(alertas_out[total_alertas].evidencia, sizeof(alertas_out[total_alertas].evidencia),
                         "pid=%d nombre=%s hijos=%d",
                         procesos[i].pid, procesos[i].nombre, procesos[i].hijos);
                total_alertas++;
            }
        }
    }

    // Detección de recursos (RAM y CPU excesivos)
    for (int i = 0; i < n_recursos; i++) {
        if (recursos[i].memoria_mb > max_memoria_mb || recursos[i].cpu_porcentaje > max_cpu) {
            if (total_alertas < max_alertas) {
                strcpy(alertas_out[total_alertas].tipo, "recursos");
                strcpy(alertas_out[total_alertas].descripcion, "Proceso con consumo anormal de recursos");
                snprintf(alertas_out[total_alertas].evidencia, sizeof(alertas_out[total_alertas].evidencia),
                         "pid=%d nombre=%s memoria_mb=%.2f cpu=%.2f",
                         recursos[i].pid, recursos[i].nombre, recursos[i].memoria_mb, recursos[i].cpu_porcentaje);
                total_alertas++;
            }
        }
    }

    // Detección de archivos (Crecimiento acelerado de almacenamiento)
    for (int i = 0; i < n_archivos; i++) {
        if (archivos[i].crecimiento_mb_s > max_crecimiento) {
            if (total_alertas < max_alertas) {
                strcpy(alertas_out[total_alertas].tipo, "archivos");
                strcpy(alertas_out[total_alertas].descripcion, "Archivo con crecimiento acelerado");
                snprintf(alertas_out[total_alertas].evidencia, sizeof(alertas_out[total_alertas].evidencia),
                         "ruta=%s crecimiento_mb_s=%.2f",
                         archivos[i].ruta, archivos[i].crecimiento_mb_s);
                total_alertas++;
            }
        }
    }

    return total_alertas; // Cantidad total de anomalías encontradas
}