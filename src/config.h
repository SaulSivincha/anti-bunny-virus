// src/config.h
#ifndef CONFIG_H
#define CONFIG_H

// Parámetros generales
#define INTERVALO_MONITOREO 1.0
#define RUTA_LOG "logs/eventos.log"
#define MODO_RESPUESTA "alerta"

// Umbrales de procesos
#define MAX_HIJOS_POR_PROCESO 20  // Límite de hijos por proceso padre
#define MAX_MEMORIA_MB 256.0  // Límite de consumo de RAM (MB)
#define MAX_CPU_PORCENTAJE 80.0  // Límite de uso de CPU (%)
#define MAX_CRECIMIENTO_ARCHIVO_MB_S 5.0  // Velocidad de llenado de ficheros
#define DIRECTORIO_MONITOREADO "/tmp/anti_bunny_demo" // Carpeta de vigilancia

#endif