#ifndef MONITOR_RECURSOS_H
#define MONITOR_RECURSOS_H

typedef struct {
    int pid;
    char nombre[256];
    float cpu_porcentaje;
    float memoria_mb;
} RecursoInfo;

int obtener_recursos(RecursoInfo *lista, int max_procesos);

#endif
