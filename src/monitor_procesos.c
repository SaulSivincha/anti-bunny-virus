#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "monitor_procesos.h"

static int es_pid(const char *nombre) {
    if (*nombre == '\0') return 0;
    for (; *nombre != '\0'; ++nombre) {
        if (!isdigit((unsigned char)*nombre)) return 0;
    }
    return 1;
}

static int leer_proceso(int pid, ProcesoInfo *proceso) {
    char ruta_stat[PATH_MAX];
    char ruta_exe[PATH_MAX];
    char linea[4096];
    FILE *archivo;
    char *cierre;
    char estado;
    struct stat datos;
    struct passwd *usuario;
    ssize_t longitud;

    memset(proceso, 0, sizeof(*proceso));
    snprintf(ruta_stat, sizeof(ruta_stat), "/proc/%d/stat", pid);
    archivo = fopen(ruta_stat, "r");
    if (archivo == NULL || fgets(linea, sizeof(linea), archivo) == NULL) {
        if (archivo != NULL) fclose(archivo);
        return -1;
    }
    fclose(archivo);

    cierre = strrchr(linea, ')');
    if (cierre == NULL || sscanf(linea, "%d (%255[^)])", &proceso->pid,
                                 proceso->nombre) != 2 ||
        sscanf(cierre + 2, "%c %d", &estado, &proceso->ppid) != 2) {
        return -1;
    }

    snprintf(ruta_stat, sizeof(ruta_stat), "/proc/%d", pid);
    if (stat(ruta_stat, &datos) == 0 && (usuario = getpwuid(datos.st_uid)) != NULL) {
        snprintf(proceso->usuario, sizeof(proceso->usuario), "%s", usuario->pw_name);
    } else {
        snprintf(proceso->usuario, sizeof(proceso->usuario), "desconocido");
    }

    snprintf(ruta_exe, sizeof(ruta_exe), "/proc/%d/exe", pid);
    longitud = readlink(ruta_exe, proceso->ruta, sizeof(proceso->ruta) - 1);
    if (longitud >= 0) {
        proceso->ruta[longitud] = '\0';
    } else {
        proceso->ruta[0] = '\0';
    }
    return 0;
}

int obtener_procesos(ProcesoInfo *lista, int max_procesos) {
    DIR *proc = opendir("/proc");
    struct dirent *entrada;
    int total = 0;

    if (proc == NULL || lista == NULL || max_procesos <= 0) return 0;
    while ((entrada = readdir(proc)) != NULL && total < max_procesos) {
        if (!es_pid(entrada->d_name)) continue;
        if (leer_proceso(atoi(entrada->d_name), &lista[total]) == 0) ++total;
    }
    closedir(proc);

    for (int i = 0; i < total; ++i) {
        lista[i].hijos = 0;
        for (int j = 0; j < total; ++j) {
            if (lista[j].ppid == lista[i].pid) ++lista[i].hijos;
        }
    }
    return total;
}
