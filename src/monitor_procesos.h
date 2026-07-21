/*
 * Interfaz del monitor de procesos. Define la muestra que describe la identidad
 * de un proceso y su proliferación, y expone la recolección desde /proc.
 */
#ifndef MONITOR_PROCESOS_H
#define MONITOR_PROCESOS_H

// Muestra de identidad, propiedad y relación padre-hijo de un proceso.
typedef struct {
    int pid;
    int ppid;
    int pgid;
    unsigned long long starttime;
    unsigned int uid;
    char nombre[256];
    char usuario[256];
    char ruta[512];
    int hijos;
    int hijos_nuevos;
    float hijos_nuevos_s;
} ProcesoInfo;

// Llena lista con hasta max_procesos muestras y devuelve cuántas fueron válidas.
int obtener_procesos(ProcesoInfo *lista, int max_procesos);

#endif
