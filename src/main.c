#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "config.h"
#include "monitor_procesos.h"
#include "monitor_recursos.h"
#include "monitor_archivos.h"
#include "motor_deteccion.h"
#include "respuesta.h"

// Variable global para manejar el bucle principal
volatile sig_atomic_t ejecutar = 1;

// Manejador de señal para capturar Ctrl+C
static void manejar_sigint(int sig) {
    (void)sig;
    ejecutar = 0;
}

int main(void) {
    // Configurar la captura de la señal SIGINT (Ctrl+C)
    struct sigaction accion = {0};
    accion.sa_handler = manejar_sigint;
    sigemptyset(&accion.sa_mask);
    sigaction(SIGINT, &accion, NULL);

    // Inicializar subsistemas
    configurar_logger(RUTA_LOG);
    printf("anti-bunny-virus iniciado en modo=%s\n", MODO_RESPUESTA);

    // Inicialización simulada de buffers de almacenamiento estáticos (para evitar fugas de memoria)
    ProcesoInfo procesos[1000];
    RecursoInfo recursos[1000];
    ArchivoInfo archivos[200];
    Alerta alertas[100];

    while (ejecutar) {
        // Recolección de datos desde los módulos recolectores
        int n_proc = obtener_procesos(procesos, 1000);
        int n_rec = obtener_recursos(recursos, 1000);
        
        // En C, pasamos la carpeta vigilada e intervalo de tiempo de forma explícita
        int n_arch = escanear_archivos(DIRECTORIO_MONITOREADO, INTERVALO_MONITOREO, archivos, 200);

        // Análisis por comportamiento en el motor de detección
        int n_alertas = detectar(
            procesos, n_proc,
            recursos, n_rec,
            archivos, n_arch,
            alertas, 100,
            MAX_HIJOS_POR_PROCESO,
            MAX_MEMORIA_MB,
            MAX_CPU_PORCENTAJE,
            MAX_CRECIMIENTO_ARCHIVO_MB_S
        );

        // Registrar y reaccionar de forma segura
        responder(alertas, n_alertas, MODO_RESPUESTA);

        // Intervalo definido en el archivo config.h
        struct timespec espera = {
            .tv_sec = (time_t)INTERVALO_MONITOREO,
            .tv_nsec = (long)((INTERVALO_MONITOREO - (time_t)INTERVALO_MONITOREO) * 1000000000.0)
        };
        nanosleep(&espera, NULL);
    }

    printf("\n[INFO] === Monitoreo finalizado de manera segura (Ctrl+C detectado) ===\n");
    return 0;
}
