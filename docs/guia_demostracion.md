# Guia de demostracion

## Preparacion

Instalar dependencias:

```bash
pip install psutil
```

Ejecutar el monitor:

```bash
python src/main.py
```

## Escenario 1: ejecucion normal

Ejecutar el monitor sin simuladores durante unos segundos.

Resultado esperado: no aparecen alertas criticas.

## Escenario 2: procesos hijos

En otra terminal:

```bash
python simuladores/simulador_fork_bomb.py
```

Resultado esperado: alerta por cantidad alta de procesos hijos.

## Escenario 3: consumo de memoria

En otra terminal:

```bash
python simuladores/simulador_memoria.py
```

Resultado esperado: alerta por consumo de memoria.

## Escenario 4: crecimiento de archivo

En otra terminal:

```bash
python simuladores/simulador_archivo.py
```

Resultado esperado: alerta por crecimiento acelerado de archivo en `/tmp/anti_bunny_demo`.

## Evidencia

Revisar:

```bash
cat logs/eventos.log
```

Cada alerta debe indicar tipo, PID o archivo, valor observado y accion tomada.
