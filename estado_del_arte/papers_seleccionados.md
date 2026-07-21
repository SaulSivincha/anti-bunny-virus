# Papers seleccionados (acceso abierto)

Esta carpeta contiene únicamente copias completas descargadas desde la editorial, la asociación académica o la institución que las publica. La selección exige una implementación y una evaluación; no se guardaron copias de sitios no autorizados ni páginas de solo resumen.

| Carpeta y paper | Aporte verificable | Rol en el proyecto |
|---|---|---|
| `gierlings_2023_isolated_and_exhausted/` | Implementa y evalúa una mitigación de *fork bomb* en Chromium; discute límites de procesos mediante cgroups en Linux. | Contención y validación de límites. |
| `yang_2008_redline_resource_control/` | Evalúa control de recursos frente a una *fork bomb* de 50 y 2 000 tareas. | Política de degradación/aislamiento antes de que el sistema quede inutilizable. |
| `pennington_2003_storage_based_intrusion_detection/` | Prototipo IDS en un servidor NFS que observa modificaciones y metadatos de archivos en tiempo real. | Base para medir crecimiento y actividad anómala de ficheros. |
| `patil_2004_i3fs_in_kernel_integrity_checker/` | Prototipo Linux en kernel para detectar actividad de sistema de archivos al acceso; reporta una evaluación de sobrecarga. | Referencia de arquitectura de monitorización de archivos robusta. |

## Fuente y cita base

1. M. Gierlings, M. Brinkmann y J. Schwenk, “Isolated and Exhausted: Attacking Operating Systems via Site Isolation in the Browser,” *USENIX Security Symposium*, 2023. [PDF oficial](https://www.usenix.org/system/files/usenixsecurity23-gierlings.pdf).
2. J. Yang, L. Ganesh, K. Kannan y M. S. Squillante, “Redline: First Class Support for Interactivity in Commodity Operating Systems,” *OSDI*, 2008. [PDF oficial](https://www.usenix.org/legacy/events/osdi08/tech/full_papers/yang/yang.pdf).
3. A. G. Pennington et al., “Storage-based Intrusion Detection: Watching Storage Activity for Suspicious Behavior,” *USENIX Security Symposium*, 2003. [PDF oficial](https://www.usenix.org/legacy/publications/library/proceedings/sec03/tech/full_papers/pennington/pennington.pdf).
4. S. Patil, A. Kashyap, G. Sivathanu y E. Zadok, “I3FS: An In-Kernel Integrity Checker and Intrusion Detection File System,” *LISA*, 2004. [PDF institucional](https://www.fsl.cs.sunysb.edu/docs/i3fs/i3fs.pdf).

## Referencia núcleo no descargada

Nakagawa y Oikawa, “Fork Bomb Attack Mitigation by Process Resource Quarantine” (CANDAR, 2016, DOI: `10.1109/CANDAR.2016.0124`) es la fuente más directamente alineada: implementa cuarentena de recursos en el kernel Linux y la evalúa. Está registrada en IEEE, pero no hay una copia completa de acceso público legítima disponible; por ello se deja como referencia para solicitar por biblioteca/autores, no se sustituye por una copia no autorizada.
