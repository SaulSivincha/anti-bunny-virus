# Paper Review: *Storage-based Intrusion Detection: Watching Storage Activity for Suspicious Behavior*

## Paper Metadata
- **Authors**: Adam G. Pennington, John D. Strunk, John L. Griffin, Craig A. N. Soules, Garth R. Goodson y Gregory R. Ganger.
- **Venue / year**: USENIX Security Symposium, 2003.
- **Domain / type**: IDS de almacenamiento; paper de sistemas con prototipo NFS.

## Executive Summary

El paper propone mover parte de la detección al servidor de almacenamiento: un IDS embebido en NFS observa peticiones y atributos persistentes aunque el SO cliente ya esté comprometido. Define señales como cambios de contenido, tamaño, tiempos, permisos, propietarios, nombres y reversión de tiempos; implementa reglas y evalúa coste y cobertura con herramientas de intrusión.

Es especialmente útil para el requisito de vigilar ficheros: demuestra que tamaño y patrones de modificación son señales viables y que el punto de observación importa. No detecta directamente una bunny virus ni atribuye cada escritura a un PID del cliente, así que no resuelve la contención que exige el proyecto.

## Summary of Contributions and Claims

1. **Taxonomía de señales observables en almacenamiento** (Secs. 2–4) para detectar manipulación persistente.
2. **IDS de almacenamiento en un servidor NFS** con reglas por fichero/directorio y generación de alertas (Sec. 5).
3. **Evaluación de cobertura y coste**: examina 18 herramientas de intrusión, de las cuales 15 muestran señales detectables; reporta 152 KB para 4 730 reglas y alrededor de 0.8% de sobrecarga en la configuración más costosa (Sec. 6).

## Strengths

### S1: Punto de observación resistente al compromiso del cliente
**Dónde**: Sec. 2. El servidor NFS puede seguir observando tras comprometerse el host. **Importancia**: enseña que un monitor anti-bunny puramente user-space necesita defensas contra su propia terminación.

### S2: Señales concretas de ficheros
**Dónde**: Secs. 3–5 y tablas de atributos. Tamaño, append, mtime/ctime, permisos y nombres se convierten en reglas, no solo en una noción vaga de “actividad anómala”.

### S3: Prototipo y métricas operacionales
**Dónde**: Sec. 6. Presenta memoria, reglas y benchmarks, además de revisar herramientas reales. Esto hace evaluable el compromiso entre cobertura y coste.

## Weaknesses

### W1: Falta de atribución al proceso responsable
**Dónde**: Sec. 5. NFS ve solicitudes de cliente, no el árbol PID, usuario y cgroup que las origina. **Mejora**: combinar eventos de filesystem con `/proc`, fanotify o eBPF en el host.

### W2: Reglas mayormente de integridad, no de velocidad
**Dónde**: Secs. 3–4. Muchas reglas detectan modificación sospechosa, no pendiente de crecimiento por ventana. **Mejora**: incorporar `Δsize/Δt`, bytes escritos y persistencia de la tasa antes de actuar.

### W3: Cobertura de malware no equivale a detección de DoS
**Dónde**: Sec. 6. Los 15/18 se refieren a cambios persistentes de herramientas, no a fork/memory bombs que quizá no escriban. **Mejora**: evaluar explícitamente procesos recursivos, archivos que crecen y cargas benignas intensivas.

## Methodology Assessment

| Criterion | Rating (1-5) | Assessment |
|---|:---:|---|
| Soundness | 4 | La visibilidad del almacenamiento respalda las reglas propuestas. |
| Novelty | 4 | Cambia el vantage point y lo materializa en NFS. |
| Reproducibility | 3 | Regla/estructura descritas, pero plataforma antigua. |
| Experimental Design | 4 | Cobertura + microbenchmarks; falta DoS/file-growth. |
| Statistical Rigor | 2 | No hay intervalos o variabilidad detallada. |
| Scalability | 3 | Memoria de reglas sí; no escala distribuida moderna. |

## Questions for the Authors
1. ¿Cómo separarían rotación legítima de logs de exfiltración o crecimiento malicioso?
2. ¿Qué latencia añade una regla con ventanas temporales?
3. ¿Cómo se correlacionaría una alerta NFS con el PID/usuario de un cliente Linux?

## Minor Issues
- Sería útil publicar la regla exacta usada por cada una de las 18 herramientas.
- La métrica de cobertura debería acompañarse de falsos positivos en operación normal.

## Literature Positioning

Frente a I3FS, el diseño se sitúa fuera del host y por ello conserva observación tras un compromiso del cliente; I3FS gana acceso a identidad local. El proyecto debe combinar el catálogo de atributos de este paper con atribución PID y una política de contención como Redline/cgroups.

## Recommendations

**Overall Assessment**: Accept. **Confidence**: High — la contribución y sus límites están bien evidenciados. **Contribution Level**: Significant.

### Actionable Suggestions for Improvement
1. Añadir evaluación con ataques de agotamiento y tasa de escritura.
2. Correlacionar solicitudes con identidad de proceso en el host.
3. Reportar FPR por cargas legítimas de logs, compilación y bases de datos.
