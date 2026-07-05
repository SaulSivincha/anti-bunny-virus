# Arquitectura, modulos y forma de trabajo

## Objetivo de la arquitectura

El sistema anti-bunny-virus se organizara por modulos para que cada integrante pueda trabajar de forma independiente, pero manteniendo una integracion clara.

La idea principal es detectar comportamiento tipo **bunny-virus / fork bomb / resource exhaustion**, observando:

- creacion acelerada de procesos;
- consumo anormal de memoria y CPU;
- crecimiento rapido de archivos;
- generacion de alertas y evidencias.

## Estructura general

```text
anti-bunny-virus/
├── src/
│   ├── main.py
│   ├── config.py
│   ├── monitor_procesos.py
│   ├── monitor_recursos.py
│   ├── monitor_archivos.py
│   ├── motor_deteccion.py
│   └── respuesta.py
├── simuladores/
│   ├── simulador_fork_bomb.py
│   ├── simulador_memoria.py
│   └── simulador_archivo.py
├── logs/
│   └── eventos.log
├── docs/
│   └── guia_demostracion.md
├── estado_del_arte/
├── estrategia_anti_bunny_virus.md
├── diseno_implementacion.md
└── arquitectura_y_pruebas.md
```

## Contenido y responsabilidad de cada modulo

### `src/main.py`

Es el punto de entrada del sistema.

Debe encargarse de:

- cargar la configuracion;
- iniciar los monitores;
- ejecutar el ciclo principal de monitoreo;
- enviar datos al motor de deteccion;
- llamar al modulo de respuesta cuando existan alertas.

No debe contener reglas complejas de deteccion. Su funcion es coordinar.

### `src/config.py`

Contiene los parametros generales del sistema.

Debe incluir:

- intervalo de monitoreo;
- limite maximo de procesos hijos;
- limite de memoria por proceso;
- limite de CPU;
- limite de crecimiento de archivos;
- directorios monitoreados;
- ruta del archivo de logs;
- modo de respuesta: `alerta` o `automatico`.

Este archivo permite modificar el comportamiento del sistema sin tocar la logica principal.

### `src/monitor_procesos.py`

Se encarga de observar los procesos activos.

Debe obtener:

- PID;
- PPID;
- nombre del proceso;
- usuario propietario;
- ruta del ejecutable;
- cantidad de procesos hijos.

Su objetivo es detectar senales de fork bomb, donde un proceso empieza a crear muchos procesos hijos en poco tiempo.

Salida esperada:

```text
lista de procesos con sus datos principales
```

### `src/monitor_recursos.py`

Se encarga de medir recursos usados por los procesos.

Debe obtener:

- uso de CPU;
- memoria usada;
- nombre del proceso;
- PID asociado.

Su objetivo es detectar procesos que consumen demasiada memoria o CPU, indicando posible agotamiento de recursos.

Salida esperada:

```text
lista de procesos con metricas de CPU y memoria
```

### `src/monitor_archivos.py`

Se encarga de vigilar archivos en directorios definidos.

Debe obtener:

- ruta del archivo;
- tamano actual;
- tamano anterior;
- tasa de crecimiento en MB/s.

Su objetivo es detectar ficheros que se llenan demasiado rapido, como indica el enunciado del proyecto.

Salida esperada:

```text
lista de archivos con su tasa de crecimiento
```

### `src/motor_deteccion.py`

Es el modulo que decide si algo es sospechoso.

Debe recibir informacion de:

- monitor de procesos;
- monitor de recursos;
- monitor de archivos.

Debe evaluar reglas como:

- demasiados procesos hijos;
- memoria mayor al umbral;
- CPU alta;
- archivo con crecimiento acelerado.

Salida esperada:

```text
lista de alertas con tipo, descripcion y evidencia
```

Este modulo es el nucleo del sistema. No debe leer procesos ni archivos directamente; solo analiza datos recibidos.

### `src/respuesta.py`

Se encarga de actuar frente a las alertas.

Debe hacer:

- mostrar alerta en consola;
- guardar alerta en `logs/eventos.log`;
- registrar fecha, tipo de alerta y evidencia;
- mantener modo seguro por defecto;
- permitir en el futuro un modo automatico para terminar procesos sospechosos.

En esta etapa, el modo recomendado es solo alerta, para evitar terminar procesos legitimos.

## Simuladores

Los simuladores sirven para demostrar el funcionamiento sin ejecutar un ataque real peligroso.

### `simuladores/simulador_fork_bomb.py`

Debe crear una cantidad limitada de procesos hijos.

Objetivo:

- demostrar deteccion de comportamiento tipo fork bomb;
- activar alerta por exceso de procesos hijos.

Debe tener limites estrictos para no saturar el sistema.

### `simuladores/simulador_memoria.py`

Debe consumir memoria progresivamente.

Objetivo:

- demostrar deteccion de consumo anormal de memoria;
- activar alerta por superar el umbral configurado.

Debe liberar recursos al finalizar el proceso.

### `simuladores/simulador_archivo.py`

Debe escribir rapidamente en un archivo temporal.

Objetivo:

- demostrar deteccion de archivos que crecen rapido;
- activar alerta por tasa de crecimiento anormal.

Debe escribir en una carpeta controlada, por ejemplo:

```text
/tmp/anti_bunny_demo
```

## Como deben trabajar los integrantes

El equipo debe trabajar por modulos, pero integrando constantemente.

### Integrante 1: procesos y recursos

Responsable de:

- `monitor_procesos.py`;
- `monitor_recursos.py`;
- pruebas de procesos hijos;
- pruebas de consumo de CPU/memoria.

Debe entregar datos en estructuras simples para que el motor de deteccion pueda usarlos.

### Integrante 2: archivos y simuladores

Responsable de:

- `monitor_archivos.py`;
- `simulador_archivo.py`;
- apoyo en `simulador_fork_bomb.py`;
- pruebas de crecimiento de ficheros.

Debe garantizar que las pruebas sean controladas y no llenen el disco.

### Integrante 3: deteccion, respuesta e integracion

Responsable de:

- `motor_deteccion.py`;
- `respuesta.py`;
- `main.py`;
- logs;
- guia de demostracion.

Debe integrar los datos de todos los modulos y generar alertas claras.

## Forma de trabajo conjunta

Para evitar conflictos, cada integrante debe trabajar en su modulo y coordinar los datos de entrada/salida.

Reglas de trabajo:

- no cambiar nombres de archivos sin avisar al equipo;
- no modificar el modulo de otro integrante sin coordinar;
- mantener funciones pequenas y faciles de probar;
- registrar cualquier cambio importante en la documentacion;
- probar cada modulo individualmente antes de integrarlo;
- usar issues de GitHub para dividir tareas;
- mover tarjetas del Project entre `Todo`, `In Progress` y `Done`.

## Integracion entre modulos

Los monitores no deben decidir si algo es virus. Solo recolectan datos.

El motor de deteccion no debe leer directamente el sistema. Solo analiza datos.

El modulo de respuesta no debe calcular metricas. Solo reporta o actua.

Flujo correcto:

```text
monitores -> motor_deteccion -> respuesta -> logs
```

## Demostracion final

La demostracion debe mostrar cuatro casos:

1. **Sistema normal**
   - Ejecutar el monitor.
   - No deben aparecer alertas importantes.

2. **Simulacion de fork bomb**
   - Ejecutar el simulador de procesos.
   - Debe aparecer alerta por exceso de hijos.

3. **Simulacion de memoria**
   - Ejecutar el simulador de memoria.
   - Debe aparecer alerta por consumo excesivo.

4. **Simulacion de archivo**
   - Ejecutar el simulador de escritura.
   - Debe aparecer alerta por crecimiento acelerado de archivo.

## Evidencia esperada

Durante la demostracion se debe mostrar:

- salida de consola del monitor;
- contenido de `logs/eventos.log`;
- proceso o archivo que activo la alerta;
- explicacion de que regla fue activada.

Ejemplo de evidencia:

```text
tipo=archivos
descripcion=Archivo con crecimiento acelerado
evidencia=ruta=/tmp/anti_bunny_demo/archivo_crecimiento_demo.bin crecimiento_mb_s=10.00
modo=alerta
```

## Criterio final de exito

El proyecto sera considerado exitoso si detecta y registra:

- creacion anormal de procesos;
- consumo anormal de memoria o CPU;
- crecimiento acelerado de archivos;
- alertas claras con evidencia suficiente.

La solucion debe mantenerse liviana: no debe escanear todo el sistema constantemente, sino monitorear por intervalos y enfocarse en senales de comportamiento anomalo.
