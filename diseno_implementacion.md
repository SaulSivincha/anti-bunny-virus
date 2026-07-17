# Diseño de implementación

## Estado actual y brecha por cerrar

La base en C ya recolecta procesos, CPU/RSS y tamaño de archivos, calcula crecimiento de archivos, registra alertas y dispone de simuladores seguros. Actualmente el motor decide con umbrales instantáneos y el alerta de archivo no tiene PID asociado. Por tanto, el producto actual es un prototipo de detección, no todavía la solución final definida en `estrategia_anti_bunny_virus.md`.

## Arquitectura objetivo

```text
recolectores (/proc y directorio)
        ↓ muestras con marca de tiempo
historial por PID / PPID / archivo
        ↓ tasas y persistencia
motor de correlación
        ↓ alerta con evidencia y confianza
atribución archivo ↔ PID (/proc/<pid>/fd)
        ↓
respuesta: alerta → pausa → terminar → cgroup
```

## Cambios técnicos planificados

### 1. Historial temporal

- Guardar muestras por PID con ticks de CPU, RSS, número de hijos y hora monotónica.
- Calcular `hijos_nuevos/s`, `ΔRSS/s`, CPU media y número de ciclos anómalos consecutivos.
- Mantener historial por ruta con tamaño, `mtime` y `Δbytes/s`.
- Eliminar entradas de procesos/archivos que ya no existan para evitar reutilizar PIDs o agotar memoria.

### 2. Motor de detección

El motor no deberá activar una alerta crítica por un único valor. Recibirá reglas configurables con: umbral, tamaño de ventana, ciclos de persistencia y severidad. Una correlación válida será, por ejemplo, proliferación de hijos y crecimiento de RSS del mismo árbol, o archivo que crece rápidamente y proceso escritor con CPU/creación de hijos anómala.

### 3. Atribución de archivos

Para cada archivo crítico se recorrerá `/proc/<pid>/fd`, se resolverán enlaces simbólicos y se asociarán coincidencias exactas con la ruta vigilada. La alerta debe incluir PID, PPID, ejecutable, usuario, ruta, bytes/s y método de atribución. Si no existe atribución, solo se registrará la alerta del archivo.

### 4. Contención segura

La interfaz de respuesta tendrá modos explícitos: `alerta`, `pausa` y `terminar`. Antes de enviar una señal debe validar PID, excluir PIDs protegidos y registrar acción y resultado. La integración con cgroups queda como mejora opcional si el entorno permite crear grupos sin privilegios indebidos.

## Configuración y evidencia

`config.h` concentrará intervalo, ventana, persistencia, umbrales, directorio vigilado, modo de respuesta y exclusiones. Todo evento contendrá marca de tiempo monotónica y de pared, tipo/severidad, métricas, PID/PPID cuando existan, ruta y acción aplicada.

## Límites explícitos

El monitor puede perder procesos efímeros entre sondeos, no siempre puede atribuir una escritura abierta y no debe usarse con modo automático fuera del entorno de prueba. Estas limitaciones deben aparecer también en el informe final.
