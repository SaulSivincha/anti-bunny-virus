# Síntesis del estado del arte y decisiones de diseño

## Propósito

La búsqueda bibliográfica está cerrada para la primera versión del proyecto. Este documento ya no es un plan de búsqueda: sintetiza qué afirma el corpus seleccionado y qué decisiones técnicas se derivan de él. Las fichas completas están en la carpeta de cada paper.

## Hallazgos relevantes

| Tema | Evidencia del corpus | Decisión para el proyecto |
|---|---|---|
| Actividad de archivos | Pennington observa tamaño, tiempos y modificaciones desde el almacenamiento; I3FS verifica atributos en tiempo de acceso. | Medir tamaño y `Δbytes/Δt` en un directorio configurado; registrar metadatos y limitar la sobrecarga. |
| Detección temprana | Los trabajos de agotamiento tratan la evolución del consumo, no solo su valor final. | Usar ventanas temporales y persistencia, en lugar de activar una alerta crítica por una muestra aislada. |
| Contención | Redline y Gierlings muestran que limitar/aislar recursos puede preservar disponibilidad ante proliferación de procesos. | Aplicar respuesta por niveles: alerta, pausa y terminación solo en laboratorio; evaluar cgroups como extensión. |
| Atribución | Los IDS de archivos no siempre conocen el proceso escritor. | Buscar coincidencias en `/proc/<pid>/fd`; si no hay certeza, alertar sin enviar señales. |
| Coste y falsos positivos | I3FS muestra que la monitorización intensiva puede ser costosa; los papers no justifican matar por una señal aislada. | Medir CPU/RSS del monitor, alertas en carga normal y exigir correlación o persistencia para acciones críticas. |

## Brecha que cubre el proyecto

Los papers seleccionados aportan componentes: monitorización de ficheros, detección de agotamiento o aislamiento de procesos. El proyecto los integra, para un entorno Linux acotado, en un flujo único:

```text
medir tasas → detectar persistencia/correlación → atribuir fichero a PID → alertar o contener con seguridad
```

La contribución esperada no es un antivirus universal ni una nueva técnica criptográfica. Es un prototipo reproducible que demuestra que esa integración detecta escenarios controlados antes de su límite seguro y conserva evidencia para actuar.

## Referencias que guían el diseño

1. Pennington et al. (2003): señales de actividad y metadatos de almacenamiento.
2. Patil et al. (2004): políticas de integridad en kernel y compromiso de rendimiento.
3. Yang et al. (2008): preservación de interactividad frente a *fork bombs* mediante control de recursos.
4. Gierlings et al. (2023): mitigación de proliferación de procesos y límites en el contexto de navegadores.
5. Nakagawa y Oikawa (2016, DOI: `10.1109/CANDAR.2016.0124`): cuarentena de recursos para *fork bombs* en Linux; referencia directa pendiente de obtener por vía institucional.

## Límites de la evidencia

El corpus no demuestra que cualquier algoritmo de umbral sea fiable para todo Linux. Por eso el informe final debe presentar el entorno, configuración, cargas, repeticiones, falsos positivos y límites de atribución; no debe extrapolar los resultados de los simuladores a todas las formas de malware.
