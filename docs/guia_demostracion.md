# Guía de demostración reproducible

## Seguridad

Ejecutar solo los simuladores incluidos; todos tienen límites. El modo predeterminado es `alerta`. `terminar_lab` está reservado a una PC dedicada y respaldada, con un usuario de laboratorio y un PGID autorizado. El monitor rechaza cualquier PID o PGID fuera de esa política.

## Preparación

```bash
make
make test
./anti-bunny-virus
```

En otra terminal se ejecuta un único simulador por vez. Antes de cada caso, guardar el commit, los parámetros de `config.h` y una copia identificable de `logs/eventos.log`.

## Casos de demostración

### 1. Línea base normal

Dejar el monitor activo durante la ventana definida sin simuladores. El resultado esperado es cero alertas críticas. Registrar alertas informativas, CPU y RSS del monitor si aparecen.

### 2. Proliferación de procesos en modo alerta

```bash
./simuladores/simulador_fork_bomb
```

El simulador crea un árbol recursivo limitado a 64 procesos y espera su finalización. La evidencia esperada incluye PID, PPID, PGID, tasa o número de hijos, severidad y tiempo de detección. No ejecutar una fork bomb ilimitada desde este repositorio.

### 3. Consumo gradual de memoria

```bash
./simuladores/simulador_memoria
```

La evidencia debe mostrar el PID, RSS, tasa de crecimiento y la razón de la alerta. El simulador libera memoria al finalizar.

### 4. Crecimiento de archivo y atribución

```bash
./simuladores/simulador_archivo
```

El monitor debe identificar la ruta con mayor bytes/s en `/tmp/anti_bunny_demo`. Si la atribución ya está implementada, el evento incluirá el PID escritor; si no se encuentra un escritor, la salida debe decirlo y limitarse a alertar.

### 5. Contención por grupo (solo después de validar los casos anteriores)

Instalar `deploy/anti-bunny-virus.service` y `deploy/anti-bunny-attack.service` en la PC dedicada. Crear `/etc/anti-bunny/lab.env` a partir de `deploy/lab.env.example`. El simulador crea su propia sesión y publica su PGID en `/run/anti-bunny/authorized_pgid`; el monitor acepta únicamente ese PGID y el usuario `antibunny-test`. La unidad del ataque usa `PIDsMax=256` como freno de emergencia, no como criterio de detección.

Ante una alerta crítica atribuida al usuario y PGID configurados, el servicio envía `SIGTERM` al grupo, espera dos segundos y usa `SIGKILL` solo si sobreviven procesos de ese mismo grupo. Verificar en el log condición crítica, PGID autorizado, señales y resultado. Detener la prueba si aparece una alerta sobre un proceso no esperado.

## Evidencia a entregar

Por cada escenario: comando, configuración, extracto del log, tiempo hasta la alerta, recursos consumidos hasta la alerta y conclusión. Los resultados se comparan entre repeticiones; una captura aislada no es evidencia suficiente.
