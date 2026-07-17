#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
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
    const char *log_env = getenv("ANTI_BUNNY_LOG");
    configurar_logger(log_env != NULL ? log_env : RUTA_LOG);
    const char *pgid_env = getenv("ANTI_BUNNY_PGID");
    const char *modo_env = getenv("ANTI_BUNNY_MODE");
    const char *pgid_file = getenv("ANTI_BUNNY_PGID_FILE");
    const char *modo = modo_env != NULL ? modo_env : MODO_RESPUESTA;
    int pgid_laboratorio = pgid_env != NULL ? atoi(pgid_env) : -1;
    configurar_laboratorio(USUARIO_LABORATORIO, pgid_laboratorio, pgid_file, SEGUNDOS_GRACIA_SIGTERM);
    printf("anti-bunny-virus iniciado en modo=%s\n", modo);

    // Inicialización simulada de buffers de almacenamiento estáticos (para evitar fugas de memoria)
    static ProcesoInfo procesos[MAX_PROCESOS_MONITOREADOS];
    static RecursoInfo recursos[MAX_PROCESOS_MONITOREADOS];
    static ArchivoInfo archivos[MAX_ARCHIVOS_MONITOREADOS];
    static Alerta alertas[MAX_ALERTAS_POR_CICLO];

    while (ejecutar) {
        // Recolección de datos desde los módulos recolectores
        int n_proc = obtener_procesos(procesos, MAX_PROCESOS_MONITOREADOS);
        int n_rec = obtener_recursos(recursos, MAX_PROCESOS_MONITOREADOS);
        
        // En C, pasamos la carpeta vigilada e intervalo de tiempo de forma explícita
        int n_arch = escanear_archivos(DIRECTORIO_MONITOREADO, INTERVALO_MONITOREO,
                                       archivos, MAX_ARCHIVOS_MONITOREADOS);
        if (n_arch > 0) {
            int mayor = 0;
            for (int i = 1; i < n_arch; ++i)
                if (archivos[i].crecimiento_mb_s > archivos[mayor].crecimiento_mb_s) mayor = i;
            if (archivos[mayor].crecimiento_mb_s > 0.0f)
                atribuir_archivo(&archivos[mayor], procesos, n_proc);
        }

        // Análisis por comportamiento en el motor de detección
        int n_alertas = detectar(
            procesos, n_proc,
            recursos, n_rec,
            archivos, n_arch,
            alertas, MAX_ALERTAS_POR_CICLO,
            MAX_HIJOS_POR_PROCESO, MAX_HIJOS_NUEVOS_POR_VENTANA, CICLOS_PERSISTENCIA_CRITICA,
            MAX_MEMORIA_MB, MAX_DELTA_MEMORIA_MB_S,
            MAX_CPU_PORCENTAJE,
            MAX_CRECIMIENTO_ARCHIVO_MB_S
        );

        // Registrar y reaccionar de forma segura
        responder(alertas, n_alertas, modo);

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
