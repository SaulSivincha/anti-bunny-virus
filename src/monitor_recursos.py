from dataclasses import dataclass
from typing import Iterable

import psutil


@dataclass(frozen=True)
class RecursoInfo:
    pid: int
    nombre: str
    cpu_porcentaje: float
    memoria_mb: float


def obtener_recursos() -> list[RecursoInfo]:
    recursos: list[RecursoInfo] = []
    for proc in psutil.process_iter(["pid", "name", "memory_info", "cpu_percent"]):
        try:
            info = proc.info
            memoria = info["memory_info"].rss / (1024 * 1024)
            recursos.append(
                RecursoInfo(
                    pid=info["pid"],
                    nombre=info.get("name") or "",
                    cpu_porcentaje=float(info.get("cpu_percent") or 0),
                    memoria_mb=memoria,
                )
            )
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue
    return recursos


def recursos_sospechosos(
    recursos: Iterable[RecursoInfo],
    max_memoria_mb: float,
    max_cpu_porcentaje: float,
) -> list[RecursoInfo]:
    return [
        recurso
        for recurso in recursos
        if recurso.memoria_mb > max_memoria_mb
        or recurso.cpu_porcentaje > max_cpu_porcentaje
    ]
