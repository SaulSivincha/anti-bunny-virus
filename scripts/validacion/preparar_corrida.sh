#!/usr/bin/env bash
# Crea una carpeta vacía y trazable para una corrida futura.
# No ejecuta ningún simulador ni modifica la configuración del sistema.

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Uso: $0 <linea-base|procesos|memoria|archivos|contencion> <repeticion>" >&2
    exit 1
fi

ESCENARIO="$1"
REPETICION="$2"

case "$ESCENARIO" in
    linea-base|procesos|memoria|archivos|contencion) ;;
    *)
        echo "Error: escenario no reconocido: $ESCENARIO" >&2
        exit 1
        ;;
esac

if [[ ! "$REPETICION" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: la repetición debe ser un entero positivo." >&2
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
MARCA_TIEMPO="$(date -u +%Y%m%dT%H%M%SZ)"
CORRIDA_DIR="$ROOT_DIR/resultados/$ESCENARIO/${MARCA_TIEMPO}-rep${REPETICION}"

umask 077
mkdir -p "$CORRIDA_DIR"

cat > "$CORRIDA_DIR/entorno.md" <<EOF
# Entorno de corrida

- Escenario: $ESCENARIO
- Repetición: $REPETICION
- Fecha UTC: $MARCA_TIEMPO
- Commit: $(git -C "$ROOT_DIR" rev-parse HEAD)
- Rama local: $(git -C "$ROOT_DIR" branch --show-current)
- Sistema: $(uname -srvmo)

## Datos de la VM por completar

- Hipervisor/proveedor:
- Identificador de snapshot inicial:
- vCPU:
- RAM:
- Disco:
- Límite de PIDs:
- Red:
- Operador de la corrida:
EOF

cp "$ROOT_DIR/src/config.h" "$CORRIDA_DIR/config.h"
cp "$ROOT_DIR/deploy/lab.env.example" "$CORRIDA_DIR/lab.env.example"

cat > "$CORRIDA_DIR/metricas.csv" <<'EOF'
escenario,repeticion,inicio_utc,alerta_inicial_s,alerta_critica_s,rss_antes_alerta_mb,cpu_monitor_pct,rss_monitor_mb,alertas_criticas,falsos_positivos,resultado,observaciones
EOF

cat > "$CORRIDA_DIR/resumen.md" <<'EOF'
# Resumen de corrida

## Resultado esperado

Pendiente de ejecución en la VM.

## Resultado observado

Pendiente.

## Evidencia incluida

- `entorno.md`
- `config.h`
- `lab.env.example`
- `metricas.csv`
- `eventos.log` (después de recolectar evidencia)
EOF

echo "$CORRIDA_DIR"
