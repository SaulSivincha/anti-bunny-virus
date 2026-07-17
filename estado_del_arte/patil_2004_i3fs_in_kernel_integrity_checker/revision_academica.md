# Paper Review: *I3FS: An In-Kernel Integrity Checker and Intrusion Detection File System*

## Paper Metadata
- **Authors**: Swapnil Patil, Anand Kashyap, Gopalan Sivathanu y Erez Zadok.
- **Venue / year**: USENIX LISA, 2004.
- **Domain / type**: seguridad de sistemas de archivos; paper de sistemas con prototipo Linux.

## Executive Summary

I3FS es un sistema de archivos apilable en kernel que intercepta accesos VFS, verifica checksums de datos/metadatos y aplica una política de bloquear o no bloquear. Su aporte central es llevar la verificación de integridad de la inspección periódica de Tripwire a la comprobación al acceso. El prototipo Linux mide coste bajo en una compilación normal (máximo 4% de tiempo transcurrido), pero costes grandes en Postmark y no aborda crecimiento anómalo ni fork bombs.

Para el proyecto es una referencia arquitectónica: muestra qué metadatos conservar, cómo hacer políticas configurables y que la frecuencia de comprobación es un compromiso explícito. No debe usarse como evidencia de que detecta una bunny virus.

## Summary of Contributions and Claims

1. **Detector de integridad en acceso** mediante capa VFS apilable (Secs. 1–2).
2. **Políticas configurables** para datos/metadatos, granularidad, frecuencia y acción BLOCK/NO-BLOCK (Sec. 2.2).
3. **Prototipo Linux evaluado** con Am-utils, Postmark y benchmark de frecuencia (Sec. 4).

## Strengths

### S1: Diseño y amenaza precisos
**Dónde**: Secs. 1–2. Define sustitución de binarios, modificaciones remotas y corrupción; enlaza cada política con atributos (tamaño, propietario, tiempos, bloques). Esto es directamente reutilizable para un monitor de ficheros.

### S2: Respuesta integrada y reversible por política
**Dónde**: Sec. 2.2. BLOCK y NO-BLOCK distinguen detección de contención; evita asumir que toda anomalía debe matar un proceso.

### S3: Evaluación revela el coste real
**Dónde**: Sec. 4. Am-utils da 4% máximo; Postmark llega a 3.5×/4.5× según política, y el experimento de frecuencia cuantifica la reducción de coste. Esa transparencia es valiosa.

## Weaknesses

### W1: Detector de integridad, no de comportamiento
**Dónde**: modelo y Sec. 2.2. Detecta cambios contra un checksum inicial, no tasa de crecimiento. **Mejora**: añadir ventanas temporales de `size`, bytes escritos y relación proceso–archivo.

### W2: Evaluación antigua y limitada
**Dónde**: Sec. 4. Ext2, kernel 2.4, una compilación y Postmark no representan ext4, SSD, contenedores ni cargas modernas. **Mejora**: repetir con fsnotify/eBPF, cgroups v2 y cargas mixtas.

### W3: Riesgo criptográfico y operacional
**Dónde**: Sec. 2 usa MD5 y bases Berkeley en kernel. MD5 ya no es adecuado para integridad resistente a adversarios y el módulo aumenta superficie de fallo. **Mejora**: SHA-256/BLAKE2, mecanismos kernel actuales y recuperación segura.

## Methodology Assessment

| Criterion | Rating (1-5) | Assessment |
|---|:---:|---|
| Soundness | 4 | Interposición y política coherentes con la amenaza declarada. |
| Novelty | 3 | Integra técnicas conocidas en una capa al acceso. |
| Reproducibility | 3 | Indica software, pero el entorno está obsoleto. |
| Experimental Design | 4 | Combina carga normal, peor caso y frecuencia. |
| Statistical Rigor | 2 | No comunica repeticiones, dispersión ni intervalos. |
| Scalability | 3 | Explora coste, no volúmenes/discos modernos. |

## Questions for the Authors
1. ¿Cómo se protege la base de checksums si el atacante obtiene privilegios kernel?
2. ¿Qué FPR/FNR tuvo frente a modificaciones legítimas y maliciosas?
3. ¿Qué diseño usarían hoy para cargas intensivas de creación de archivos?

## Minor Issues
- La conclusión “4%” debe acompañarse siempre del caso Postmark, mucho peor.
- Faltan mediciones de latencia por acceso y de uso de memoria de las bases.

## Literature Positioning

El paper se posiciona frente a Tripwire, AIDE, Samhain, LIDS y LSM. Complementa el IDS de almacenamiento de Pennington: I3FS observa desde el host/kernel y Pennington desde NFS. Para anti-bunny-virus aporta la capa de observación, no el clasificador ni la respuesta por PID.

## Recommendations

**Overall Assessment**: Weak Accept. **Confidence**: High — las limitaciones están bien delimitadas por el propio alcance. **Contribution Level**: Moderate.

### Actionable Suggestions for Improvement
1. Reimplementar sobre interfaces Linux actuales y hash moderno.
2. Añadir detección temporal de crecimiento y atribución a proceso.
3. Publicar repeticiones, dispersión y evaluación de ataques reales.
