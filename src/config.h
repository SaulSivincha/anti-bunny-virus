#ifndef CONFIG_H
#define CONFIG_H

/*
 * Umbrales e intervalos del monitor anti-bunny-virus.
 * Ajustar aquí el comportamiento sin tocar la lógica de detección.
 */

#define INTERVALO_MONITOREO 0.25
#define RUTA_LOG "logs/eventos.log"
#define MODO_RESPUESTA "alerta"
#define USUARIO_LABORATORIO "antibunny-test"

#define MAX_HIJOS_POR_PROCESO 20
#define MAX_MEMORIA_MB 256.0
#define MAX_CPU_PORCENTAJE 80.0
#define MAX_CRECIMIENTO_ARCHIVO_MB_S 5.0
#define MAX_DELTA_MEMORIA_MB_S 32.0
#define MAX_HIJOS_NUEVOS_POR_VENTANA 8
#define CICLOS_PERSISTENCIA_CRITICA 2
#define SEGUNDOS_GRACIA_SIGTERM 2
#define DIRECTORIO_MONITOREADO "/tmp/anti_bunny_demo"
#define MAX_PROCESOS_MONITOREADOS 8192
#define MAX_ARCHIVOS_MONITOREADOS 1024
#define MAX_ALERTAS_POR_CICLO 256

#endif
