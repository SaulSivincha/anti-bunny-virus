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
exigir src/respuesta.c '!a->accionable' \
    'las alertas no accionables son rechazadas'
exigir src/respuesta.c 'strcmp\(a->severidad, "critica"\) != 0' \
    'solo las alertas críticas pueden solicitar contención'
exigir src/respuesta.c 'strcmp\(a->tipo, "procesos"\) != 0' \
    'solo alertas de proliferación son contenibles'
exigir src/respuesta.c 'a->pid <= 1' \
    'PID 1 y PID inválidos quedan excluidos'
exigir src/respuesta.c 'a->pgid <= 1' \
    'PGID inválidos quedan excluidos'
exigir src/respuesta.c 'starttime_actual == a->starttime' \
    'la identidad revalida starttime antes de actuar'
exigir src/respuesta.c 'pgid_actual == a->pgid' \
    'la identidad revalida PGID antes de actuar'
exigir src/respuesta.c 'kill\(-a->pgid, SIGTERM\)' \
    'la primera señal de contención es SIGTERM al PGID de la alerta'
exigir src/respuesta.c 'kill\(-a->pgid, SIGKILL\)' \
    'SIGKILL solo está disponible como escalamiento al mismo PGID'
if grep -q 'pgid_autorizado' "$ROOT_DIR/src/respuesta.c" 2>/dev/null; then
    printf 'ERROR: respuesta.c aún referencia pgid_autorizado\n' >&2
    ERRORES=$((ERRORES + 1))
else
    printf 'OK: no hay lista blanca pgid_autorizado\n'
fi

if grep -q 'uid_laboratorio' "$ROOT_DIR/src/respuesta.c" 2>/dev/null; then
    printf 'ERROR: respuesta.c aún referencia uid_laboratorio\n' >&2
    ERRORES=$((ERRORES + 1))
else
    printf 'OK: no hay filtro por UID de laboratorio\n'
fi

exigir deploy/anti-bunny-attack.service '^User=antibunny-test$' \
    'el simulador se ejecuta sin privilegios'
exigir deploy/anti-bunny-attack.service '^PIDsMax=256$' \
    'el servicio de ataque conserva un freno de emergencia'
exigir deploy/anti-bunny-attack.service '^KillMode=control-group$' \
    'systemd gestiona el grupo completo del simulador'
exigir deploy/lab.env.example '^ANTI_BUNNY_MODE=terminar_lab$' \
    'el ejemplo de despliegue habilita contención'

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
