# Guia de demostracion

## Preparacion

El proyecto se ejecuta en Linux y requiere un compilador C compatible con C11 y `make`.

```bash
make
make test
```

El primer comando genera el monitor `anti-bunny-virus` y los tres simuladores dentro de `simuladores/`.

## Escenario 1: ejecucion normal

En una terminal, iniciar el monitor:

```bash
./anti-bunny-virus
```

Dejarlo ejecutarse durante unos segundos. No deben aparecer alertas criticas. Presionar `Ctrl+C` para cerrar el monitor de forma segura.

## Escenario 2: procesos hijos

Con el monitor activo, ejecutar en otra terminal:

```bash
./simuladores/simulador_fork_bomb
```

El simulador crea como maximo 25 hijos temporales y espera a que terminen. Debe producirse una alerta de procesos para su proceso padre.

## Escenario 3: consumo de memoria

Con el monitor activo, ejecutar:

```bash
./simuladores/simulador_memoria
```

El simulador reserva hasta 320 MB de forma gradual y libera la memoria al finalizar. Debe producirse una alerta de recursos al superar el umbral de memoria configurado.

## Escenario 4: crecimiento de archivo

Con el monitor activo, ejecutar:

```bash
./simuladores/simulador_archivo
```

El simulador escribe un maximo de 16 MB en `/tmp/anti_bunny_demo/archivo_crecimiento_demo.bin`. Debe producirse una alerta por crecimiento acelerado de archivo.

## Evidencia

Revisar las alertas y el registro persistente:

```bash
cat logs/eventos.log
```

Cada alerta incluye tipo, evidencia y modo de respuesta. El modo predeterminado es `alerta`; no termina procesos. El modo `automatico` solo debe habilitarse en un entorno controlado.
