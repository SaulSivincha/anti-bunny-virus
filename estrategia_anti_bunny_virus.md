# Estrategia del sistema anti-bunny-virus

## Problema

Un *bunny-virus* se aborda aquí como un ataque de agotamiento de recursos: un proceso puede proliferar, consumir memoria o CPU, o hacer crecer un fichero hasta degradar la disponibilidad de Linux. El riesgo no es solamente perder espacio o rendimiento; cuando los recursos se agotan, ya puede no ser posible ejecutar la orden de recuperación.

El proyecto no busca reconocer malware por firma ni afirmar que todo fichero grande es malicioso. Busca detectar **comportamiento anómalo sostenido** y contenerlo de manera trazable antes del colapso.

## Pregunta e hipótesis

**Pregunta de investigación.** ¿Un monitor en C para Linux que use tasas temporales de procesos, recursos y crecimiento de archivos puede detectar de forma temprana escenarios controlados de agotamiento y contener de forma segura el proceso atribuible?

**Hipótesis.** Frente a reglas basadas solo en un umbral instantáneo, combinar tasas medidas en ventanas temporales, persistencia de la anomalía y al menos dos señales relacionadas reducirá alertas falsas y permitirá detectar los simuladores antes de su límite seguro.

La hipótesis se evaluará con tiempo de detección, recursos consumidos antes de responder, tasa de falsos positivos y sobrecarga del monitor. No se afirmará protección contra malware real fuera de los escenarios evaluados.

## Fundamento en la literatura

- Pennington et al. muestran que tamaño, tiempos y modificaciones de archivos son señales útiles de intrusión; esto justifica registrar bytes/s y metadatos.
- Patil et al. muestran la utilidad de políticas en tiempo de acceso, pero también el coste de monitorizar intensivamente; esto justifica limitar el alcance a directorios configurados y medir sobrecarga.
- Yang et al. y Gierlings et al. muestran que aislar o limitar recursos puede preservar la disponibilidad ante *fork bombs*; esto justifica una respuesta gradual, no una terminación ciega.

Las revisiones y transcripciones están en `estado_del_arte/<paper>/`.

## Alcance del producto final

El monitor deberá:

1. observar `/proc` para construir relaciones PID–PPID y medir CPU/RSS;
2. medir por intervalo la tasa de creación de hijos, crecimiento de memoria y crecimiento de archivos en bytes/s;
3. identificar el fichero de mayor crecimiento dentro del directorio vigilado;
4. intentar atribuir un fichero anómalo a procesos que lo tengan abierto mediante `/proc/<pid>/fd`;
5. emitir alertas con evidencia suficiente para reproducir la decisión;
6. aplicar una respuesta gradual y segura en laboratorio.

Quedan fuera del alcance: clasificación por firma, protección de todo el kernel, análisis de tráfico de red y garantía de detener malware no atribuible.

## Política de decisión y respuesta

Una señal aislada crea una alerta informativa. Una alerta crítica exige persistencia durante una ventana configurada o correlación de dos señales del mismo proceso o árbol. La respuesta se ordena por riesgo:

1. `alerta`: registrar y mostrar evidencia; modo por defecto.
2. `pausa`: `SIGSTOP` únicamente para un PID atribuible, no protegido y en laboratorio.
3. `terminar`: `SIGTERM` solo tras cumplir la política crítica y con lista de exclusión.
4. `cgroup` (extensión): limitar procesos/CPU/memoria del grupo en lugar de terminarlo.

Nunca se actuará automáticamente sobre PID 1, procesos del kernel, el propio monitor ni procesos sin evidencia atribuible.

## Criterio de éxito

El proyecto estará terminado cuando los tres simuladores controlados sean detectados antes de su límite, la carga normal no produzca alertas críticas, cada decisión quede registrada con evidencia y `make && make test` permita reproducir el sistema y sus pruebas.
