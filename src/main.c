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

/* Bandera que mantiene activo el ciclo principal hasta recibir Ctrl+C. */
volatile sig_atomic_t ejecutar = 1;

/* Función para detener el monitor de forma segura al pulsar Ctrl+C. */
static void manejar_sigint(int sig) {
    (void)sig;
    ejecutar = 0;
}

/* Punto de entrada: coordina recolectores, detección y respuesta en un bucle periódico. */
int main(void) {
    struct sigaction accion = {0};
    accion.sa_handler = manejar_sigint;
    sigemptyset(&accion.sa_mask);
    sigaction(SIGINT, &accion, NULL);

    /* Rutas y política de laboratorio pueden sobreescribirse con variables de entorno. */
    const char *log_env = getenv("ANTI_BUNNY_LOG");
    configurar_logger(log_env != NULL ? log_env : RUTA_LOG);
    const char *pgid_env = getenv("ANTI_BUNNY_PGID");
    const char *modo_env = getenv("ANTI_BUNNY_MODE");
    const char *pgid_file = getenv("ANTI_BUNNY_PGID_FILE");
    const char *modo = modo_env != NULL ? modo_env : MODO_RESPUESTA;
    int pgid_laboratorio = pgid_env != NULL ? atoi(pgid_env) : -1;
    configurar_laboratorio(USUARIO_LABORATORIO, pgid_laboratorio, pgid_file, SEGUNDOS_GRACIA_SIGTERM);
    printf("anti-bunny-virus iniciado en modo=%s\n", modo);

    /* Buffers estáticos reutilizados cada ciclo para no reservar memoria en heap. */
    static ProcesoInfo procesos[MAX_PROCESOS_MONITOREADOS];
    static RecursoInfo recursos[MAX_PROCESOS_MONITOREADOS];
    static ArchivoInfo archivos[MAX_ARCHIVOS_MONITOREADOS];
    static Alerta alertas[MAX_ALERTAS_POR_CICLO];

    while (ejecutar) {
        int n_proc = obtener_procesos(procesos, MAX_PROCESOS_MONITOREADOS);
        int n_rec = obtener_recursos(recursos, MAX_PROCESOS_MONITOREADOS);
        
        int n_arch = escanear_archivos(DIRECTORIO_MONITOREADO, INTERVALO_MONITOREO,
                                       archivos, MAX_ARCHIVOS_MONITOREADOS);
        /* Solo se intenta atribuir el fichero que más crece en el intervalo. */
        if (n_arch > 0) {
            int mayor = 0;
            for (int i = 1; i < n_arch; ++i)
                if (archivos[i].crecimiento_mb_s > archivos[mayor].crecimiento_mb_s) mayor = i;
            if (archivos[mayor].crecimiento_mb_s > 0.0f)
                atribuir_archivo(&archivos[mayor], procesos, n_proc);
        }

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

        responder(alertas, n_alertas, modo);

        struct timespec espera = {
            .tv_sec = (time_t)INTERVALO_MONITOREO,
            .tv_nsec = (long)((INTERVALO_MONITOREO - (time_t)INTERVALO_MONITOREO) * 1000000000.0)
        };
        nanosleep(&espera, NULL);
    }

    printf("\n[INFO] Monitoreo finalizado de manera segura (Ctrl+C detectado)\n");
    return 0;
}
