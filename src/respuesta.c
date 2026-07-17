#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "respuesta.h"

static char ruta_archivo_log[512] = "logs/eventos.log";

static void asegurar_directorio_log(const char *ruta_log) {
    char directorio[512];
    char *barra;
    snprintf(directorio, sizeof(directorio), "%s", ruta_log);
    barra = strrchr(directorio, '/');
    if (barra != NULL) {
        *barra = '\0';
        if (*directorio != '\0') mkdir(directorio, 0755);
    }
}

void configurar_logger(const char* ruta_log) {
    // Copiar la ruta a nuestra variable estática
    strncpy(ruta_archivo_log, ruta_log, sizeof(ruta_archivo_log) - 1);
    ruta_archivo_log[sizeof(ruta_archivo_log) - 1] = '\0';
    asegurar_directorio_log(ruta_archivo_log);
    // Registrar el mensaje de inicio del daemon en el log (Criterio de Aceptación)
    FILE* archivo = fopen(ruta_archivo_log, "a");
    if (archivo != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char marca_tiempo[26];
        
        strftime(marca_tiempo, sizeof(marca_tiempo), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(archivo, "%s INFO anti-bunny-virus iniciado en modo=alerta\n", marca_tiempo);
        fclose(archivo);
    }
}

void responder(Alerta* alertas, int n_alertas, const char* modo) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char marca_tiempo[26];
    
    // Formatear fecha y hora igual al formato estándar de logs
    strftime(marca_tiempo, sizeof(marca_tiempo), "%Y-%m-%d %H:%M:%S", tm_info);

    // Abrir el archivo en modo append (agregar al final)
    FILE* archivo = fopen(ruta_archivo_log, "a");

    for (int i = 0; i < n_alertas; i++) {
        // Impresión obligatoria en consola
        printf("%s WARNING tipo=%s descripcion=\"%s\" evidencia=\"%s\" modo=%s\n",
               marca_tiempo, alertas[i].tipo, alertas[i].descripcion, alertas[i].evidencia, modo);

        // Guardar la alerta en el log persistente
        if (archivo != NULL) {
            fprintf(archivo, "%s WARNING tipo=%s descripcion=\"%s\" evidencia=\"%s\" modo=%s\n",
                    marca_tiempo, alertas[i].tipo, alertas[i].descripcion, alertas[i].evidencia, modo);
        }

        // Estructura para el soporte del modo automático
        if (strcmp(modo, "automatico") == 0) {
            if (alertas[i].pid > 1) {
                if (kill(alertas[i].pid, SIGTERM) == 0) {
                    printf("%s INFO pid=%d accion=SIGTERM\n", marca_tiempo, alertas[i].pid);
                } else {
                    perror("No se pudo terminar el proceso sospechoso");
                }
            }
        }
    }

    if (archivo != NULL) {
        fclose(archivo);
    }
}
