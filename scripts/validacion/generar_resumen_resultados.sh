#!/usr/bin/env bash
# Genera una tabla Markdown a partir de metricas.csv reales.

set -euo pipefail
export LC_ALL=C

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RESULTADOS_DIR="${1:-$ROOT_DIR/resultados}"
TEMP_DIR="$(mktemp -d)"
DATOS="$TEMP_DIR/datos.csv"
trap 'rm -rf "$TEMP_DIR"' EXIT

if [[ ! -d "$RESULTADOS_DIR" ]]; then
    echo "Error: no existe el directorio de resultados: $RESULTADOS_DIR" >&2
    exit 1
fi

while IFS= read -r -d '' archivo; do
    awk 'NR > 1 && NF' "$archivo" >> "$DATOS"
done < <(find "$RESULTADOS_DIR" -type f -name metricas.csv -print0 | sort -z)

if [[ ! -s "$DATOS" ]]; then
    echo "Error: no hay filas de métricas reales para resumir." >&2
    exit 1
fi

mediana() {
    local escenario="$1"
    local columna="$2"
    awk -F, -v e="$escenario" -v c="$columna" '$1 == e && $c != "" {print $c}' "$DATOS" |
        sort -n |
        awk '{v[NR]=$1} END {if (NR == 0) print "-"; else if (NR % 2) print v[(NR+1)/2]; else printf "%.3f", (v[NR/2]+v[NR/2+1])/2}'
}

echo '# Resumen experimental'
echo
echo '| Escenario | Corridas | Mediana alerta inicial (s) | Mediana alerta crítica (s) | Mediana CPU monitor (%) | Mediana RSS monitor (MB) | Falsos positivos |'
echo '|---|---:|---:|---:|---:|---:|---:|'
for escenario in linea-base procesos memoria archivos contencion; do
    CANTIDAD="$(awk -F, -v e="$escenario" '$1 == e {n++} END {print n + 0}' "$DATOS")"
    FALSOS="$(awk -F, -v e="$escenario" '$1 == e && $10 != "" {s += $10} END {print s + 0}' "$DATOS")"
    printf '| %s | %s | %s | %s | %s | %s | %s |\n' \
        "$escenario" "$CANTIDAD" "$(mediana "$escenario" 4)" \
        "$(mediana "$escenario" 5)" "$(mediana "$escenario" 7)" \
        "$(mediana "$escenario" 8)" "$FALSOS"
done

echo
echo '> Tabla generada exclusivamente desde los archivos `metricas.csv` disponibles.'
