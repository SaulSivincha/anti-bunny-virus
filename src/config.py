from dataclasses import dataclass, field
from pathlib import Path


@dataclass(frozen=True)
class Config:
    intervalo_monitoreo: float = 1.0
    max_hijos_por_proceso: int = 20
    max_memoria_mb: float = 256.0
    max_cpu_porcentaje: float = 80.0
    max_crecimiento_archivo_mb_s: float = 5.0
    modo_respuesta: str = "alerta"
    directorios_monitoreados: tuple[Path, ...] = field(
        default_factory=lambda: (Path("/tmp/anti_bunny_demo"),)
    )
    ruta_log: Path = Path("logs/eventos.log")


CONFIG = Config()
