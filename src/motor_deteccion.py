from dataclasses import dataclass

from monitor_archivos import ArchivoInfo, archivos_sospechosos
from monitor_procesos import ProcesoInfo, procesos_sospechosos_por_hijos
from monitor_recursos import RecursoInfo, recursos_sospechosos


@dataclass(frozen=True)
class Alerta:
    tipo: str
    descripcion: str
    evidencia: str


def detectar(
    procesos: list[ProcesoInfo],
    recursos: list[RecursoInfo],
    archivos: list[ArchivoInfo],
    max_hijos: int,
    max_memoria_mb: float,
    max_cpu_porcentaje: float,
    max_crecimiento_archivo_mb_s: float,
) -> list[Alerta]:
    alertas: list[Alerta] = []

    for proceso in procesos_sospechosos_por_hijos(procesos, max_hijos):
        alertas.append(
            Alerta(
                tipo="procesos",
                descripcion="Proceso con demasiados hijos",
                evidencia=f"pid={proceso.pid} nombre={proceso.nombre} hijos={proceso.hijos}",
            )
        )

    for recurso in recursos_sospechosos(
        recursos, max_memoria_mb, max_cpu_porcentaje
    ):
        alertas.append(
            Alerta(
                tipo="recursos",
                descripcion="Proceso con consumo anormal de recursos",
                evidencia=(
                    f"pid={recurso.pid} nombre={recurso.nombre} "
                    f"memoria_mb={recurso.memoria_mb:.2f} "
                    f"cpu={recurso.cpu_porcentaje:.2f}"
                ),
            )
        )

    for archivo in archivos_sospechosos(
        archivos, max_crecimiento_archivo_mb_s
    ):
        alertas.append(
            Alerta(
                tipo="archivos",
                descripcion="Archivo con crecimiento acelerado",
                evidencia=(
                    f"ruta={archivo.ruta} "
                    f"crecimiento_mb_s={archivo.crecimiento_mb_s:.2f}"
                ),
            )
        )

    return alertas
