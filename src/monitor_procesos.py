from dataclasses import dataclass
from typing import Iterable

import psutil


@dataclass(frozen=True)
class ProcesoInfo:
    pid: int
    ppid: int
    nombre: str
    usuario: str
    ruta: str
    hijos: int


def obtener_procesos() -> list[ProcesoInfo]:
    procesos: list[ProcesoInfo] = []
    for proc in psutil.process_iter(["pid", "ppid", "name", "username", "exe"]):
        try:
            hijos = len(proc.children(recursive=False))
            info = proc.info
            procesos.append(
                ProcesoInfo(
                    pid=info["pid"],
                    ppid=info["ppid"],
                    nombre=info.get("name") or "",
                    usuario=info.get("username") or "",
                    ruta=info.get("exe") or "",
                    hijos=hijos,
                )
            )
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue
    return procesos


def procesos_sospechosos_por_hijos(
    procesos: Iterable[ProcesoInfo], max_hijos: int
) -> list[ProcesoInfo]:
    return [proceso for proceso in procesos if proceso.hijos > max_hijos]
