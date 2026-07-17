#!/usr/bin/env bash
# Verifica requisitos locales antes de preparar una corrida experimental.
# No inicia el monitor, simuladores ni modos de contención.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "Error: las pruebas del proyecto requieren Linux." >&2
    exit 1
fi

for comando in gcc make git; do
    if ! command -v "$comando" >/dev/null 2>&1; then
        echo "Error: falta el comando requerido: $comando" >&2
        exit 1
    fi
done

if [[ ! -r /proc/self/stat ]]; then
    echo "Error: /proc no está disponible; el monitor no puede ejecutarse." >&2
    exit 1
fi

cd "$ROOT_DIR"
make
make test

echo "Entorno local verificado."
echo "Repositorio: $ROOT_DIR"
echo "Kernel: $(uname -r)"
echo "Commit: $(git rev-parse --short HEAD)"
echo "Nota: este script no certifica que se esté en una VM; la VM se documenta al preparar la corrida."
