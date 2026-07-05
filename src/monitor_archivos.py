from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class ArchivoInfo:
    ruta: Path
    tamano_bytes: int
    crecimiento_mb_s: float


class MonitorArchivos:
    def __init__(self) -> None:
        self._tamano_anterior: dict[Path, int] = {}

    def escanear(self, directorios: tuple[Path, ...], intervalo: float) -> list[ArchivoInfo]:
        archivos: list[ArchivoInfo] = []
        for directorio in directorios:
            directorio.mkdir(parents=True, exist_ok=True)
            for ruta in directorio.rglob("*"):
                if not ruta.is_file():
                    continue
                tamano = ruta.stat().st_size
                anterior = self._tamano_anterior.get(ruta, tamano)
                crecimiento = max(tamano - anterior, 0) / (1024 * 1024) / intervalo
                self._tamano_anterior[ruta] = tamano
                archivos.append(
                    ArchivoInfo(
                        ruta=ruta,
                        tamano_bytes=tamano,
                        crecimiento_mb_s=crecimiento,
                    )
                )
        return archivos


def archivos_sospechosos(
    archivos: list[ArchivoInfo], max_crecimiento_mb_s: float
) -> list[ArchivoInfo]:
    return [
        archivo
        for archivo in archivos
        if archivo.crecimiento_mb_s > max_crecimiento_mb_s
    ]
