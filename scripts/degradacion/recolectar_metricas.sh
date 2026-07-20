#!/usr/bin/env bash

set -u

SALIDA="${1:-metricas_degradacion.csv}"
INTERVALO="${INTERVALO:-0.25}"

inicio_ns="$(date +%s%N)"

echo "elapsed_ms,timestamp_utc,procesos,cpu_pct,ram_usada_mb,ram_pct,load1,pid_pct,psi_cpu,psi_memoria,responde" > "$SALIDA"

read_cpu() {
    awk '/^cpu / {
        idle=$5+$6
        total=0
        for(i=2;i<=NF;i++) total+=$i
        print total, idle
    }' /proc/stat
}

read total_anterior idle_anterior < <(read_cpu)

while true; do
    ahora_ns="$(date +%s%N)"
    elapsed_ms=$(( (ahora_ns - inicio_ns) / 1000000 ))
    timestamp="$(date -u +"%Y-%m-%dT%H:%M:%S.%3NZ")"

    procesos="$(find /proc -maxdepth 1 -type d -name '[0-9]*' 2>/dev/null | wc -l)"

    read total_actual idle_actual < <(read_cpu)

    delta_total=$((total_actual - total_anterior))
    delta_idle=$((idle_actual - idle_anterior))

    if (( delta_total > 0 )); then
        cpu_pct="$(awk -v t="$delta_total" -v i="$delta_idle" \
            'BEGIN {printf "%.2f", 100*(t-i)/t}')"
    else
        cpu_pct="0.00"
    fi

    total_anterior="$total_actual"
    idle_anterior="$idle_actual"

    read mem_total_kb mem_disponible_kb < <(
        awk '
            /MemTotal:/ {total=$2}
            /MemAvailable:/ {available=$2}
            END {print total, available}
        ' /proc/meminfo
    )

    ram_usada_mb="$(awk -v t="$mem_total_kb" -v a="$mem_disponible_kb" \
        'BEGIN {printf "%.2f", (t-a)/1024}')"

    ram_pct="$(awk -v t="$mem_total_kb" -v a="$mem_disponible_kb" \
        'BEGIN {printf "%.2f", 100*(t-a)/t}')"

    load1="$(awk '{print $1}' /proc/loadavg)"

    pid_limite="$(ulimit -u)"
    if [[ "$pid_limite" =~ ^[0-9]+$ ]] && (( pid_limite > 0 )); then
        pid_pct="$(awk -v p="$procesos" -v l="$pid_limite" \
            'BEGIN {printf "%.2f", 100*p/l}')"
    else
        pid_pct="0.00"
    fi

    psi_cpu="$(awk '/^some / {
        for(i=1;i<=NF;i++)
            if($i ~ /^avg10=/) {
                split($i,a,"=");
                print a[2];
                exit
            }
    }' /proc/pressure/cpu 2>/dev/null || echo 0)"

    psi_memoria="$(awk '/^some / {
        for(i=1;i<=NF;i++)
            if($i ~ /^avg10=/) {
                split($i,a,"=");
                print a[2];
                exit
            }
    }' /proc/pressure/memory 2>/dev/null || echo 0)"

    echo "$elapsed_ms,$timestamp,$procesos,$cpu_pct,$ram_usada_mb,$ram_pct,$load1,$pid_pct,$psi_cpu,$psi_memoria,1" >> "$SALIDA"

    sleep "$INTERVALO"
done
