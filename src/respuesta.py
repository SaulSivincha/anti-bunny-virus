import logging
from pathlib import Path

from motor_deteccion import Alerta


def configurar_logger(ruta_log: Path) -> logging.Logger:
    ruta_log.parent.mkdir(parents=True, exist_ok=True)
    logger = logging.getLogger("anti_bunny_virus")
    logger.setLevel(logging.INFO)
    logger.handlers.clear()

    formato = logging.Formatter("%(asctime)s %(levelname)s %(message)s")

    archivo = logging.FileHandler(ruta_log)
    archivo.setFormatter(formato)
    logger.addHandler(archivo)

    consola = logging.StreamHandler()
    consola.setFormatter(formato)
    logger.addHandler(consola)

    return logger


def responder(alertas: list[Alerta], logger: logging.Logger, modo: str) -> None:
    for alerta in alertas:
        logger.warning(
            "tipo=%s descripcion=%s evidencia=%s modo=%s",
            alerta.tipo,
            alerta.descripcion,
            alerta.evidencia,
            modo,
        )
