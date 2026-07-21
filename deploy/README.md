# Despliegue en VM dedicada

Este despliegue es para una PC o VM Linux dedicada, respaldada y con consola local.

## Componentes

- `anti-bunny-virus.service` ejecuta el monitor (requiere privilegios para enviar señales a grupos de procesos).
- `anti-bunny-attack.service` ejecuta el simulador recursivo con el usuario `antibunny-test` y `PIDsMax=256` como freno de emergencia.

## Contención

En modo `terminar_lab`, ante una alerta **crítica accionable** de tipo `procesos`, el monitor envía señales al **PGID indicado en la alerta** (`kill(-pgid, …)`), tras revalidar PID, PGID y `starttime` en `/proc`. No usa una lista blanca de PGID publicada por el simulador.

## Configuración

1. Crear el usuario sin privilegios `antibunny-test` en la VM.
2. Instalar el binario y simuladores en `/opt/anti-bunny-virus` y copiar las unidades a la ruta de unidades de `systemd`.
3. Crear `/etc/anti-bunny/lab.env` desde `lab.env.example`:

   ```text
   ANTI_BUNNY_MODE=terminar_lab
   ```

4. Recargar `systemd`, iniciar primero el monitor y después la unidad de ataque.

## Verificación

El log debe mostrar una alerta crítica de `procesos`, seguida de `SIGTERM_GRUPO` al PGID de la alerta y, solo si el grupo sigue vivo tras la gracia, `SIGKILL_GRUPO`. Las alertas no críticas, PID/PGID reservados o identidades obsoletas deben registrar `accion=denegada`.

El valor `PIDsMax=256` es una red de seguridad del servicio de ataque. Una demostración satisfactoria termina el grupo detectado antes de llegar al límite.
