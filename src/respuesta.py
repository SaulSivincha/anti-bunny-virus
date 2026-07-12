# src/respuesta.py
import logging
from pathlib import Path

from motor_deteccion import Alerta


def configurar_logger(ruta_log: Path) -> logging.Logger:
    """Configura el logger hacia consola y archivo, previniendo duplicidad."""
    ruta_log.parent.mkdir(parents=True, exist_ok=True)
    logger = logging.getLogger("anti_bunny_virus")
    logger.setLevel(logging.INFO)
    
    # Evita duplicación excesiva de handlers si se inicializa más de una vez
    logger.handlers.clear()

    formato = logging.Formatter("%(asctime)s %(levelname)s %(message)s")

    # Handler para archivo (logs/eventos.log)
    archivo = logging.FileHandler(ruta_log)
    archivo.setFormatter(formato)
    logger.addHandler(archivo)

    # Handler para consola
    consola = logging.StreamHandler()
    consola.setFormatter(formato)
    logger.addHandler(consola)

    return logger


def responder(alertas: list[Alerta], logger: logging.Logger, modo: str) -> None:
    """Registra las alertas y prepara el terreno para la mitigación futura."""
    for alerta in alertas:
        # Registrar siempre la alerta con formato estructurado (Consola y Archivo)
        logger.warning(
            "tipo=%s descripcion=%s evidencia=%s modo=%s",
            alerta.tipo,
            alerta.descripcion,
            alerta.evidencia,
            modo,
        )
        
        # Preparar la estructura para un modo futuro automático
        if modo == "automatico":
            # Ejemplo futuro: psutil.Process(pid).kill() o pkill
            logger.info("[MITIGACIÓN MULTI-MODO] Estructura lista para terminación automática del proceso.")
            
        elif modo == "alerta":
            # En modo alerta, no se termina ningún proceso (Garantía de seguridad inicial)
            pass