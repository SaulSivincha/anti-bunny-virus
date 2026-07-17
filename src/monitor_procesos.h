#ifndef MONITOR_PROCESOS_H
#define MONITOR_PROCESOS_H

typedef struct {
    int pid;
    int ppid;
    char nombre[256];
    char usuario[256];
    char ruta[512];
    int hijos;
} ProcesoInfo;

int obtener_procesos(ProcesoInfo *lista, int max_procesos);

#endif
