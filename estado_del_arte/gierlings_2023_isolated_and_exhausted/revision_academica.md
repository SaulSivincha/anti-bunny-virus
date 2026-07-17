# Paper Review: *Isolated and Exhausted: Attacking Operating Systems via Site Isolation in the Browser*

## Paper Metadata
- **Authors**: Matthias Gierlings, Marcus Brinkmann y Jörg Schwenk.
- **Venue / year**: USENIX Security Symposium, 2023.
- **Domain / type**: seguridad de sistemas; paper empírico de ataques y mitigaciones.

## Executive Summary

El trabajo muestra que el aislamiento por sitio de los navegadores incrementa la cantidad de procesos y sockets del SO hasta abrir dos vectores de agotamiento: una *fork bomb* inducida por JavaScript y DEMONS, agotamiento de puertos UDP. No propone un detector host-wide: prueba ataques en navegadores y propone límites preventivos en el navegador, incluida una modificación implementada en Chromium.

La aportación es significativa para delimitar la amenaza y para diseñar contención, pero su evidencia se limita al contexto browser/Site Isolation. Para anti-bunny-virus es una fuente fuerte para la respuesta (límite y aislamiento), no para justificar una heurística general de crecimiento de archivos.

## Summary of Contributions and Claims

1. **Site Isolation permite inducir una fork bomb desde contenido web** (Secs. 3–4 y apéndice C); la evaluación compara navegadores y SO.
2. **DEMONS puede agotar puertos UDP mediante el navegador** (Secs. 5–6); los autores hacen pruebas de Internet y de laboratorio.
3. **Un límite de procesos visibles por pestaña/ventana mitiga la fork bomb** (Sec. 7.2): implementan el cambio en Chromium y lo validan con Top 1000 de Tranco y una prueba de falso positivo.

## Strengths

### S1: Amenaza y mecanismo reproducibles
**Dónde**: Secs. 3–4 y apéndice C. **Qué**: separa los niveles de agotamiento y explica la creación de procesos. **Por qué importa**: traduce “fork bomb” en señales medibles (tasa de creación y cuenta de procesos).

### S2: Mitigación implementada y evaluada
**Dónde**: Sec. 7.2. La modificación de Chromium impone el límite y se prueba contra el ataque y tráfico benigno; es evidencia más útil que una recomendación conceptual.

### S3: Evaluación con escenarios distintos
**Dónde**: Secs. 4 y 6. Incluye laboratorio e Internet para DEMONS, y explicita plataforma, versiones y apéndices, favoreciendo repetición.

## Weaknesses

### W1: Alcance limitado al navegador
**Dónde**: Sec. 7. La solución controla procesos visibles del browser, no identifica ni atribuye procesos arbitrarios del host. **Impacto**: no basta para el objetivo del proyecto. **Mejora**: medir un monitor de SO que agrupe árbol de procesos/cgroup y ejecute contención reversible.

### W2: No compara detectores de fork bomb
**Dónde**: Secs. 4 y 7. Se miden efectos y límite, pero no precisión, latencia de alarma ni FPR de una regla de detección. **Mejora**: reportar tiempo hasta detección, procesos creados antes de respuesta y carga benigna variada.

### W3: Generalización temporal y de plataforma acotada
**Dónde**: Secs. 4, 6 y apéndice. Versiones concretas de navegadores/SO condicionan el resultado. **Mejora**: repetir en versiones actuales, hardware heterogéneo y configuraciones cgroup modernas.

## Methodology Assessment

| Criterion | Rating (1-5) | Assessment |
|---|:---:|---|
| Soundness | 4 | El mecanismo y la contramedida están bien ligados a la amenaza. |
| Novelty | 4 | Conecta Site Isolation con agotamiento del SO. |
| Reproducibility | 4 | Describe versiones y remite a artefactos; faltan todos los datos crudos. |
| Experimental Design | 4 | Ataque, laboratorio, Internet y falso positivo; falta host-wide baseline. |
| Statistical Rigor | 3 | Reporta resultados, pero no intervalos ni análisis estadístico sistemático. |
| Scalability | 3 | Prueba Top 1000, pero solo una familia de aplicaciones. |

## Questions for the Authors

1. ¿Cuál es la latencia y el coste de memoria de imponer el límite bajo navegación prolongada?
2. ¿Qué valor de límite resulta seguro en equipos con distintas RAM/CPU?
3. ¿Cómo cambiarían los resultados con cgroups v2 como contención host-side?

## Minor Issues

- Conviene separar más claramente en tablas los resultados del ataque y los de la mitigación.
- Publicar series por plataforma facilitaría comparar umbrales.

## Literature Positioning

El artículo cita la cuarentena de recursos de Nakagawa y Oikawa y la descripción de fork bombs de Berlot y Sang. Aporta una variante moderna y remota del vector; se complementa con Redline para control de recursos general y con el monitor del proyecto para atribución a PID/archivo.

## Recommendations

**Overall Assessment**: Accept. **Confidence**: High — el paper provee implementación y evaluación, aunque su alcance es específico. **Contribution Level**: Significant.

### Actionable Suggestions for Improvement
1. Medir detección y falso positivo además de prevención.
2. Evaluar cgroups/pids.max y respuesta reversible a nivel del SO.
3. Liberar trazas y scripts completos de las evaluaciones.
