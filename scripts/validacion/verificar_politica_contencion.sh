#!/usr/bin/env bash
# Comprueba invariantes estáticas de la política de contención.
# No inicia servicios ni envía señales.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ERRORES=0

exigir() {
    local archivo="$1"
    local patron="$2"
    local descripcion="$3"
    if grep -Eq "$patron" "$ROOT_DIR/$archivo"; then
        printf 'OK: %s\n' "$descripcion"
    else
        printf 'ERROR: %s (%s)\n' "$descripcion" "$archivo" >&2
        ERRORES=$((ERRORES + 1))
    fi
}

exigir src/config.h '^#define MODO_RESPUESTA "alerta"$' \
    'el modo predeterminado solo registra alertas'
exigir src/config.h '^#define USUARIO_LABORATORIO "antibunny-test"$' \
    'el usuario de laboratorio está fijado'
exigir src/respuesta.c '!a->accionable' \
    'las alertas no accionables son rechazadas'
exigir src/respuesta.c 'strcmp\(a->severidad, "critica"\) != 0' \
    'solo las alertas críticas pueden solicitar contención'
exigir src/respuesta.c 'a->pid <= 1' \
    'PID 1 y PID inválidos quedan excluidos'
exigir src/respuesta.c 'a->pgid <= 1' \
    'PGID inválidos quedan excluidos'
exigir src/respuesta.c 'a->pgid != pgid_autorizado' \
    'la política rechaza PGID no autorizado'
exigir src/respuesta.c 'a->uid != uid_laboratorio' \
    'la política rechaza UID no autorizado'
exigir src/respuesta.c 'starttime_actual == a->starttime' \
    'la identidad revalida starttime antes de actuar'
exigir src/respuesta.c 'pgid_actual == a->pgid' \
    'la identidad revalida PGID antes de actuar'
exigir src/respuesta.c 'datos.st_uid == uid_laboratorio' \
    'la identidad revalida UID antes de actuar'
exigir src/respuesta.c 'kill\(-a->pgid, SIGTERM\)' \
    'la primera señal de contención es SIGTERM al grupo'
exigir src/respuesta.c 'kill\(-a->pgid, SIGKILL\)' \
    'SIGKILL solo está disponible como escalamiento al mismo grupo'
exigir deploy/anti-bunny-attack.service '^User=antibunny-test$' \
    'el simulador se ejecuta sin privilegios'
exigir deploy/anti-bunny-attack.service '^PIDsMax=256$' \
    'el servicio de ataque conserva un freno de emergencia'
exigir deploy/anti-bunny-attack.service '^KillMode=control-group$' \
    'systemd gestiona el grupo completo del simulador'
exigir deploy/lab.env.example '^ANTI_BUNNY_PGID=-1$' \
    'el PGID no queda autorizado por defecto'
exigir deploy/lab.env.example '^ANTI_BUNNY_PGID_FILE=/run/anti-bunny/authorized_pgid$' \
    'la autorización se publica por una ruta conocida'

LINEA_TERM="$(grep -n 'kill(-a->pgid, SIGTERM)' "$ROOT_DIR/src/respuesta.c" | head -n 1 | cut -d: -f1 || true)"
LINEA_KILL="$(grep -n 'kill(-a->pgid, SIGKILL)' "$ROOT_DIR/src/respuesta.c" | head -n 1 | cut -d: -f1 || true)"
if [[ -n "$LINEA_TERM" && -n "$LINEA_KILL" && "$LINEA_TERM" -lt "$LINEA_KILL" ]]; then
    printf 'OK: SIGTERM aparece antes que SIGKILL\n'
else
    printf 'ERROR: no se pudo comprobar el orden SIGTERM -> SIGKILL\n' >&2
    ERRORES=$((ERRORES + 1))
fi

if [[ "$ERRORES" -ne 0 ]]; then
    printf 'Política estática inválida: %d comprobaciones fallaron.\n' "$ERRORES" >&2
    exit 1
fi

printf 'Política estática de contención: OK\n'
