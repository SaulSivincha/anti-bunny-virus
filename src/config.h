/*
 * Configuración general del monitor. Centraliza el intervalo de muestreo,
 * los umbrales de detección, la política predeterminada y los límites de los
 * arreglos usados durante cada ciclo.
 */
#ifndef CONFIG_H
#define CONFIG_H

// Parámetros generales
#define INTERVALO_MONITOREO 0.25
#define RUTA_LOG "logs/eventos.log"
#define MODO_RESPUESTA "alerta"
#define USUARIO_LABORATORIO "antibunny-test"

// Umbrales usados por el motor de detección.
#define MAX_HIJOS_POR_PROCESO 20  // Límite de hijos por proceso padre
#define MAX_MEMORIA_MB 256.0  // Límite de consumo de RAM (MB)
#define MAX_CPU_PORCENTAJE 80.0  // Límite de uso de CPU (%)
#define MAX_CRECIMIENTO_ARCHIVO_MB_S 5.0  // Velocidad de llenado de ficheros
#define MAX_DELTA_MEMORIA_MB_S 32.0  // Aumento máximo de RSS por segundo
#define MAX_HIJOS_NUEVOS_POR_VENTANA 8  // Proliferación observada entre muestras
#define CICLOS_PERSISTENCIA_CRITICA 2  // Ciclos anómalos para escalar severidad
#define SEGUNDOS_GRACIA_SIGTERM 2  // Espera antes de considerar SIGKILL
#define DIRECTORIO_MONITOREADO "/tmp/anti_bunny_demo" // Carpeta de vigilancia
// Límites de los buffers estáticos; evitan crecimiento interno sin control.
#define MAX_PROCESOS_MONITOREADOS 8192
#define MAX_ARCHIVOS_MONITOREADOS 1024
#define MAX_ALERTAS_POR_CICLO 256

#endif
