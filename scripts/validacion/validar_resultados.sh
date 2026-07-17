#!/usr/bin/env bash
# Valida evidencia real generada por las corridas de la issue #6.
# Sin --completo, la ausencia de resultados es válida para desarrollo/CI.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RESULTADOS_DIR="$ROOT_DIR/resultados"
MODO_COMPLETO=0

if [[ ${1:-} == "--completo" ]]; then
    MODO_COMPLETO=1
    shift
fi
if [[ $# -gt 1 ]]; then
    echo "Uso: $0 [--completo] [directorio-resultados]" >&2
    exit 1
fi
if [[ $# -eq 1 ]]; then
    RESULTADOS_DIR="$(cd "$1" && pwd)"
fi

ESCENARIOS=(linea-base procesos memoria archivos contencion)
ARCHIVOS=(entorno.md config.h lab.env.example metricas.csv resumen.md eventos.log)
CABECERA='escenario,repeticion,inicio_utc,alerta_inicial_s,alerta_critica_s,rss_antes_alerta_mb,cpu_monitor_pct,rss_monitor_mb,alertas_criticas,falsos_positivos,resultado,observaciones'
ERRORES=0
TOTAL=0
declare -A REPETICIONES

if [[ ! -d "$RESULTADOS_DIR" ]]; then
    if [[ "$MODO_COMPLETO" -eq 1 ]]; then
        echo "Error: no existe el directorio de resultados: $RESULTADOS_DIR" >&2
        exit 1
    fi
    echo "Sin resultados locales: validación de estructura omitida."
    exit 0
fi

shopt -s nullglob
for escenario in "${ESCENARIOS[@]}"; do
    REPETICIONES["$escenario"]=0
    corridas=("$RESULTADOS_DIR/$escenario"/*-rep*)
    for corrida in "${corridas[@]}"; do
        [[ -d "$corrida" ]] || continue
        TOTAL=$((TOTAL + 1))
        REPETICIONES["$escenario"]=$((REPETICIONES["$escenario"] + 1))
        for archivo in "${ARCHIVOS[@]}"; do
            if [[ ! -s "$corrida/$archivo" ]]; then
                echo "Error: falta $archivo en $corrida" >&2
                ERRORES=$((ERRORES + 1))
            fi
        done
        if [[ -s "$corrida/metricas.csv" ]]; then
            if [[ "$(head -n 1 "$corrida/metricas.csv")" != "$CABECERA" ]]; then
                echo "Error: cabecera CSV inválida en $corrida" >&2
                ERRORES=$((ERRORES + 1))
            fi
            FILAS="$(awk 'NR > 1 && NF {n++} END {print n + 0}' "$corrida/metricas.csv")"
            if [[ "$MODO_COMPLETO" -eq 1 && "$FILAS" -lt 1 ]]; then
                echo "Error: no hay métricas observadas en $corrida" >&2
                ERRORES=$((ERRORES + 1))
            fi
        fi
        if [[ "$MODO_COMPLETO" -eq 1 ]]; then
            if grep -Eq 'Pendiente( de ejecución)?\.?$' "$corrida/resumen.md"; then
                echo "Error: resumen pendiente en $corrida" >&2
                ERRORES=$((ERRORES + 1))
            fi
            if grep -q '^No se encontró logs/eventos.log' "$corrida/eventos.log"; then
                echo "Error: la corrida no contiene un log real en $corrida" >&2
                ERRORES=$((ERRORES + 1))
            fi
        fi
    done
done

if [[ "$MODO_COMPLETO" -eq 1 ]]; then
    for escenario in "${ESCENARIOS[@]}"; do
        if [[ "${REPETICIONES[$escenario]}" -lt 3 ]]; then
            echo "Error: $escenario tiene ${REPETICIONES[$escenario]} corridas; se requieren 3." >&2
            ERRORES=$((ERRORES + 1))
        fi
    done
fi

if [[ "$ERRORES" -ne 0 ]]; then
    echo "Evidencia inválida: $ERRORES errores en $TOTAL corridas." >&2
    exit 1
fi

echo "Evidencia válida: $TOTAL corridas revisadas."
