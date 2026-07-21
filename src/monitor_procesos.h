#ifndef MONITOR_PROCESOS_H
#define MONITOR_PROCESOS_H

/* Datos de un proceso leídos desde /proc para detectar proliferación. */
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

int obtener_procesos(ProcesoInfo *lista, int max_procesos);

#endif
