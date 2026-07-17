# Despliegue de laboratorio

Este despliegue es exclusivo para una PC Linux dedicada, respaldada y con consola local. No instala ni ejecuta una fork bomb ilimitada.

## Componentes

- `anti-bunny-virus.service` ejecuta el monitor separado del ataque.
- `anti-bunny-attack.service` ejecuta el simulador recursivo con el usuario `antibunny-test` y `PIDsMax=256` como freno de emergencia.
- `/run/anti-bunny/authorized_pgid` contiene el PGID publicado por el simulador. El monitor solo puede enviar señales a ese grupo y a ese usuario.

## Configuración

1. Crear manualmente el usuario sin privilegios `antibunny-test` en la PC dedicada.
2. Instalar el binario y simuladores en `/opt/anti-bunny-virus` y copiar las unidades a la ruta de unidades de `systemd` de la distribución.
3. Crear `/etc/anti-bunny/lab.env` desde `lab.env.example`, con:

   ```text
   ANTI_BUNNY_MODE=terminar_lab
   ANTI_BUNNY_PGID_FILE=/run/anti-bunny/authorized_pgid
   ```

4. Recargar `systemd`, iniciar primero el monitor y después la unidad de ataque.

## Verificación

El log debe mostrar una alerta crítica de `procesos`, seguida de `SIGTERM_GRUPO` y, solo si el grupo sigue vivo tras la gracia, `SIGKILL_GRUPO`. Si el usuario, el PGID o la severidad no coinciden, debe aparecer `accion=denegada` y no se envía ninguna señal.

El valor `PIDsMax=256` es una red de seguridad: una demostración satisfactoria termina el grupo antes de llegar al límite. La comparación de colapso sin defensa se registra como video separado y requiere recuperación local; no es parte del ejecutable ni de esta unidad.
