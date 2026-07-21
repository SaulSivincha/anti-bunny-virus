/*
 * Interfaz del monitor de recursos. Define las métricas de CPU y memoria de un
 * proceso y la operación que obtiene una muestra completa desde /proc.
 */
#ifndef MONITOR_RECURSOS_H
#define MONITOR_RECURSOS_H

// Muestra de consumo asociada a una identidad PID-starttime.
typedef struct {
    int pid;
    unsigned long long starttime;
    char nombre[256];
    float cpu_porcentaje;
    float memoria_mb;
    float delta_memoria_mb_s;
} RecursoInfo;

// Recolecta hasta max_procesos métricas y devuelve el número de muestras válidas.
int obtener_recursos(RecursoInfo *lista, int max_procesos);

#endif
