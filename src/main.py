# src/main.py
import time
import sys  # Requerido para sys.exit()

from config import CONFIG
from monitor_archivos import MonitorArchivos
from monitor_procesos import obtener_procesos
from monitor_recursos import obtener_recursos
from motor_deteccion import detectar
from respuesta import configurar_logger, responder


def main() -> None:
    logger = configurar_logger(CONFIG.ruta_log)
    monitor_archivos = MonitorArchivos()
    logger.info("anti-bunny-virus iniciado en modo=%s", CONFIG.modo_respuesta)

    try:
        while True:
            procesos = obtener_procesos()
            recursos = obtener_recursos()
            archivos = monitor_archivos.escanear(
                CONFIG.directorios_monitoreados,
                CONFIG.intervalo_monitoreo,
            )
            alertas = detectar(
                procesos=procesos,
                recursos=recursos,
                archivos=archivos,
                max_hijos=CONFIG.max_hijos_por_proceso,
                max_memoria_mb=CONFIG.max_memoria_mb,
                max_cpu_porcentaje=CONFIG.max_cpu_porcentaje,
                max_crecimiento_archivo_mb_s=CONFIG.max_crecimiento_archivo_mb_s,
            )
            responder(alertas, logger, CONFIG.modo_respuesta)
            time.sleep(CONFIG.intervalo_monitoreo)
            
    except KeyboardInterrupt:
        # Captura Ctrl+C y cierra el programa limpiamente sin lanzar tracebacks
        print("\n")
        logger.info("=== Monitoreo finalizado por el usuario de forma segura (Ctrl+C) ===")
        sys.exit(0)


if __name__ == "__main__":
    main()