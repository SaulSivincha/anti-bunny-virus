#!/usr/bin/env bash
# Añade evidencia local a una carpeta creada por preparar_corrida.sh.
# No inicia ni termina procesos; solo copia archivos y consulta metadatos.

set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Uso: $0 <directorio-de-corrida>" >&2
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RESULTADOS_DIR="$ROOT_DIR/resultados"
CORRIDA_DIR="$(cd "$1" && pwd)"

case "$CORRIDA_DIR" in
    "$RESULTADOS_DIR"/*) ;;
    *)
        echo "Error: la corrida debe estar dentro de $RESULTADOS_DIR" >&2
        exit 1
        ;;
esac

if [[ ! -f "$CORRIDA_DIR/entorno.md" ]]; then
    echo "Error: no parece una corrida creada por preparar_corrida.sh" >&2
    exit 1
fi

if [[ -f "$ROOT_DIR/logs/eventos.log" ]]; then
    cp "$ROOT_DIR/logs/eventos.log" "$CORRIDA_DIR/eventos.log"
else
    printf 'No se encontró logs/eventos.log al recolectar la evidencia.\n' > "$CORRIDA_DIR/eventos.log"
fi

{
    echo
    echo "## Recolección"
    echo
    echo "- Fecha UTC: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "- Commit al recolectar: $(git -C "$ROOT_DIR" rev-parse HEAD)"
    echo "- Log copiado: eventos.log"
} >> "$CORRIDA_DIR/entorno.md"

echo "Evidencia recolectada en: $CORRIDA_DIR"
