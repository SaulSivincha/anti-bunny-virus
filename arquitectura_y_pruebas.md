# Arquitectura, responsabilidades y protocolo de pruebas

## Estructura

```text
src/           monitor y módulos C
simuladores/   cargas seguras y acotadas
tests/         pruebas unitarias y de integración
logs/          evidencia generada en ejecución
docs/          guía de demostración
estado_del_arte/<paper>/  PDF, transcripción y revisión
```

Los recolectores no toman decisiones; `motor_deteccion` correlaciona las muestras y `respuesta` aplica únicamente la acción que autorice la política. `main` coordina el ciclo, el intervalo y el cierre seguro con `SIGINT`.

## Trabajo por integrante

| Integrante | Entregables | Criterio de aceptación |
|---|---|---|
| 1. Procesos y recursos | historial PID/PPID, tasas de hijos, RSS y CPU; limpieza de muestras caducadas | pruebas con procesos efímeros, memoria gradual y carga CPU; sin reutilizar datos de PID muerto |
| 2. Archivos y atribución | crecimiento bytes/s, selección del archivo máximo y búsqueda de escritores en `/proc/<pid>/fd` | simulador de archivo atribuido correctamente; caso sin escritor no genera una acción destructiva |
| 3. Decisión, respuesta e integración | correlación, severidad, exclusiones, modos de respuesta, logs y automatización | `make test`; cada alerta crítica tiene evidencia y las señales solo se aplican a PIDs permitidos |

La integración se realiza mediante PRs pequeños: un cambio funcional, prueba asociada, revisión y ejecución de `make && make test` antes de fusionar.

## Matriz experimental

| Caso | Carga controlada | Resultado esperado | Métricas |
|---|---|---|---|
| Normal | monitor sin simuladores y actividad ordinaria | cero alertas críticas | alertas/hora, CPU y RSS del monitor |
| Procesos | simulador de fork limitado | alerta temprana para padre/árbol | tiempo de detección, hijos antes de alerta |
| Memoria | reserva gradual y acotada | alerta de crecimiento/saturación | tiempo de detección, RSS antes de alerta |
| Archivo | escritura limitada en directorio vigilado | ruta con máximo bytes/s y PID si está disponible | bytes escritos antes de alerta, exactitud de atribución |
| Contención | simulador en modo laboratorio | `SIGTERM` al PGID y `SIGKILL` solo si sobrevive | acción correcta, recuperación y ausencia de daño colateral |

## Protocolo reproducible

1. Compilar con `make` y ejecutar `make test`.
2. Limpiar o identificar el log de cada ejecución y anotar configuración, fecha, commit y entorno Linux.
3. Ejecutar cada escenario al menos tres veces con parámetros iguales.
4. Conservar logs y calcular mediana de tiempo de detección, recursos consumidos y número de alertas.
5. Informar resultados negativos y falsos positivos; no seleccionar solo la mejor corrida.

## Definition of Done

- Las pruebas unitarias cubren reglas de una señal, correlación, persistencia, exclusiones y ausencia de atribución.
- Los tres simuladores se detectan antes de alcanzar su límite seguro.
- El caso normal no emite alertas críticas durante la ventana definida.
- El log permite reconstruir qué señal produjo una decisión y qué acción se ejecutó.
- La guía de demostración y el informe distinguen resultados medidos de trabajo futuro.
