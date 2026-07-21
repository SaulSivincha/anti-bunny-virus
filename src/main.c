/*
 * Punto de entrada de anti-bunny-virus. Configura el laboratorio y coordina en
 * cada ciclo la recolección, la detección, el registro y la respuesta segura.
 */
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

// sig_atomic_t permite cambiar la bandera de forma segura desde SIGINT.
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

    // Las variables de entorno sobrescriben los valores predeterminados.
    const char *log_env = getenv("ANTI_BUNNY_LOG");
    configurar_logger(log_env != NULL ? log_env : RUTA_LOG);
    const char *pgid_env = getenv("ANTI_BUNNY_PGID");
    const char *modo_env = getenv("ANTI_BUNNY_MODE");
    const char *pgid_file = getenv("ANTI_BUNNY_PGID_FILE");
    const char *modo = modo_env != NULL ? modo_env : MODO_RESPUESTA;
    int pgid_laboratorio = pgid_env != NULL ? atoi(pgid_env) : -1;
    configurar_laboratorio(USUARIO_LABORATORIO, pgid_laboratorio, pgid_file, SEGUNDOS_GRACIA_SIGTERM);
    printf("anti-bunny-virus iniciado en modo=%s\n", modo);

    // Los buffers estáticos fijan el máximo de datos procesados en cada ciclo.
    static ProcesoInfo procesos[MAX_PROCESOS_MONITOREADOS];
    static RecursoInfo recursos[MAX_PROCESOS_MONITOREADOS];
    static ArchivoInfo archivos[MAX_ARCHIVOS_MONITOREADOS];
    static Alerta alertas[MAX_ALERTAS_POR_CICLO];

    while (ejecutar) {
        // Obtener una fotografía actual de procesos y recursos.
        int n_proc = obtener_procesos(procesos, MAX_PROCESOS_MONITOREADOS);
        int n_rec = obtener_recursos(recursos, MAX_PROCESOS_MONITOREADOS);
        
        // Medir el crecimiento de los archivos del directorio vigilado.
        int n_arch = escanear_archivos(DIRECTORIO_MONITOREADO, INTERVALO_MONITOREO,
                                       archivos, MAX_ARCHIVOS_MONITOREADOS);
        if (n_arch > 0) {
            // Solo se intenta atribuir el archivo con mayor tasa positiva.
            int mayor = 0;
            for (int i = 1; i < n_arch; ++i)
                if (archivos[i].crecimiento_mb_s > archivos[mayor].crecimiento_mb_s) mayor = i;
            if (archivos[mayor].crecimiento_mb_s > 0.0f)
                atribuir_archivo(&archivos[mayor], procesos, n_proc);
        }

        // Aplicar umbrales y persistencia a las tres clases de muestras.
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

        // Registrar siempre; contener únicamente si la política lo autoriza.
        responder(alertas, n_alertas, modo);

        // Mantener la frecuencia de muestreo configurada.
        struct timespec espera = {
            .tv_sec = (time_t)INTERVALO_MONITOREO,
            .tv_nsec = (long)((INTERVALO_MONITOREO - (time_t)INTERVALO_MONITOREO) * 1000000000.0)
        };
        nanosleep(&espera, NULL);
    }

    printf("\n[INFO] === Monitoreo finalizado de manera segura (Ctrl+C detectado) ===\n");
    return 0;
}
