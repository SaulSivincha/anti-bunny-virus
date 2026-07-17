#ifndef MONITOR_RECURSOS_H
#define MONITOR_RECURSOS_H

typedef struct {
    int pid;
    unsigned long long starttime;
    char nombre[256];
    float cpu_porcentaje;
    float memoria_mb;
    float delta_memoria_mb_s;
} RecursoInfo;

int obtener_recursos(RecursoInfo *lista, int max_procesos);

#endif
