# Arquitectura, modulos y forma de trabajo

## Objetivo de la arquitectura

El sistema anti-bunny-virus se organizara por modulos en Lenguaje C para que cada integrante pueda trabajar de forma independiente, pero manteniendo una integracion clara, con interaccion nativa con el sistema operativo Linux.

La idea principal es detectar comportamiento tipo **bunny-virus / fork bomb / resource exhaustion**, observando:

- creacion acelerada de procesos;
- consumo anormal de memoria y CPU;
- crecimiento rapido de archivos;
- generacion de alertas y evidencias.

## Estructura general

```text
anti-bunny-virus/
├── src/
│   ├── main.c
│   ├── config.h
│   ├── monitor_procesos.c
│   ├── monitor_procesos.h
│   ├── monitor_recursos.c
│   ├── monitor_recursos.h
│   ├── monitor_archivos.c
│   ├── monitor_archivos.h
│   ├── motor_deteccion.c
│   ├── motor_deteccion.h
│   ├── respuesta.c
│   └── respuesta.h
├── simuladores/
│   ├── simulador_fork_bomb.c
│   ├── simulador_memoria.c
│   └── simulador_archivo.c
├── logs/
│   └── eventos.log
├── docs/
│   └── guia_demostracion.md
├── estado_del_arte/
├── tests/
│   └── test_motor_deteccion.c
├── Makefile
├── estrategia_anti_bunny_virus.md
├── diseno_implementacion.md
└── arquitectura_y_pruebas.md
```

## Contenido y responsabilidad de cada modulo

### `src/main.c`

Es el punto de entrada del sistema.

Debe encargarse de:

- cargar la configuracion desde `config.h`;
- iniciar los monitores mediante llamadas a sus funciones de inicializacion;
- ejecutar el ciclo principal de monitoreo (`while` controlado por intervalo, usando `sleep()` o `nanosleep()`);
- capturar la senal `SIGINT` (Ctrl+C) mediante `signal()` o `sigaction()` para cerrar el programa de forma segura, liberando recursos y cerrando el log;
- enviar datos al motor de deteccion;
- llamar al modulo de respuesta cuando existan alertas.

No debe contener reglas complejas de deteccion. Su funcion es coordinar.

### `src/config.h`

Contiene los parametros generales del sistema, definidos como constantes de preprocesador (`#define`) o mediante una `struct Config`.

Debe incluir:

- intervalo de monitoreo (en segundos);
- limite maximo de procesos hijos;
- limite de memoria por proceso (en KB, segun `/proc/[pid]/status`);
- limite de CPU (porcentaje estimado);
- limite de crecimiento de archivos (MB/s);
- directorios monitoreados (rutas como `char*`);
- ruta del archivo de logs;
- modo de respuesta: `ALERTA` o `AUTOMATICO` (enum).

Este archivo permite modificar el comportamiento del sistema sin tocar la logica principal.

### `src/monitor_procesos.h` / `src/monitor_procesos.c`

Se encarga de observar los procesos activos leyendo directamente el sistema de archivos virtual **`/proc`**.

Debe obtener, recorriendo `/proc/[pid]/`:

- PID (nombre del directorio numerico dentro de `/proc`);
- PPID (leido de `/proc/[pid]/stat`);
- nombre del proceso (leido de `/proc/[pid]/comm`);
- usuario propietario (mediante `stat()` sobre `/proc/[pid]` y `getpwuid()`);
- ruta del ejecutable (mediante `readlink()` sobre `/proc/[pid]/exe`);
- cantidad de procesos hijos (contabilizada al agrupar procesos por su PPID).

Su objetivo es detectar senales de fork bomb, donde un proceso empieza a crear muchos procesos hijos en poco tiempo.

Funciones principales sugeridas:

```c
int leer_procesos(ProcesoInfo *lista, int max_procesos);
int contar_hijos(pid_t ppid, ProcesoInfo *lista, int total);
```

Salida esperada:

```text
arreglo de estructuras ProcesoInfo con los datos principales
```

### `src/monitor_recursos.h` / `src/monitor_recursos.c`

Se encarga de medir recursos usados por los procesos, leyendo `/proc/[pid]/stat` y `/proc/[pid]/status`.

Debe obtener:

- uso de CPU (calculado a partir de los campos `utime`/`stime` de `/proc/[pid]/stat` y el tiempo transcurrido entre lecturas);
- memoria usada (campo `VmRSS` de `/proc/[pid]/status`);
- nombre del proceso;
- PID asociado.

Su objetivo es detectar procesos que consumen demasiada memoria o CPU, indicando posible agotamiento de recursos.

Funciones principales sugeridas:

```c
int leer_recursos(pid_t pid, RecursoInfo *info);
double calcular_uso_cpu(RecursoInfo *actual, RecursoInfo *anterior, double intervalo);
```

Salida esperada:

```text
arreglo de estructuras RecursoInfo con metricas de CPU y memoria
```

### `src/monitor_archivos.h` / `src/monitor_archivos.c`

Se encarga de vigilar archivos en directorios definidos, usando `<dirent.h>` para recorrer directorios y `<sys/stat.h>` para obtener metadatos.

Debe obtener:

- ruta del archivo (concatenada al recorrer el directorio con `opendir()`/`readdir()`);
- tamano actual (campo `st_size` de `struct stat`, obtenido con `stat()`);
- tamano anterior (guardado en memoria entre ciclos de monitoreo);
- tasa de crecimiento en MB/s (calculada como diferencia de tamano entre ciclos, dividida entre el intervalo).

Su objetivo es detectar ficheros que se llenan demasiado rapido, como indica el enunciado del proyecto.

Funciones principales sugeridas:

```c
int escanear_directorio(const char *ruta, ArchivoInfo *lista, int max_archivos);
double calcular_crecimiento(ArchivoInfo *actual, ArchivoInfo *anterior, double intervalo);
```

Salida esperada:

```text
arreglo de estructuras ArchivoInfo con su tasa de crecimiento
```

### `src/motor_deteccion.h` / `src/motor_deteccion.c`

Es el modulo que decide si algo es sospechoso.

Debe recibir informacion (arreglos de estructuras en memoria) de:

- monitor de procesos;
- monitor de recursos;
- monitor de archivos.

Debe evaluar reglas como:

- demasiados procesos hijos;
- memoria mayor al umbral;
- CPU alta;
- archivo con crecimiento acelerado.

Funcion principal sugerida:

```c
int evaluar_alertas(ProcesoInfo *procesos, RecursoInfo *recursos,
                     ArchivoInfo *archivos, AlertaInfo *alertas, int max_alertas);
```

Salida esperada:

```text
arreglo de estructuras AlertaInfo con tipo, descripcion y evidencia
```

Este modulo es el nucleo del sistema. No debe leer `/proc` ni directorios directamente; solo analiza datos recibidos en estructuras ya construidas por los monitores.

### `src/respuesta.h` / `src/respuesta.c`

Se encarga de actuar frente a las alertas.

Debe hacer:

- mostrar alerta en consola (`printf`/`fprintf` sobre `stderr`);
- guardar alerta en `logs/eventos.log` usando `fopen(..., "a")` y `fprintf()`;
- registrar fecha (mediante `<time.h>`, `time()` y `strftime()`), tipo de alerta y evidencia;
- mantener modo seguro por defecto;
- permitir en el futuro un modo automatico para terminar procesos sospechosos mediante `kill(pid, SIGTERM)`.

En esta etapa, el modo recomendado es solo alerta, para evitar terminar procesos legitimos.

Funciones principales sugeridas:

```c
void registrar_alerta(const AlertaInfo *alerta, FILE *log);
void responder(const AlertaInfo *alerta, int modo);
```

## Simuladores

Los simuladores sirven para demostrar el funcionamiento sin ejecutar un ataque real peligroso. Se compilan como binarios independientes a partir de sus fuentes en C.

### `simuladores/simulador_fork_bomb.c`

Debe crear una cantidad limitada de procesos hijos usando `fork()` en un bucle acotado.

Objetivo:

- demostrar deteccion de comportamiento tipo fork bomb;
- activar alerta por exceso de procesos hijos.

Debe tener limites estrictos (contador maximo de `fork()`) y llamar a `wait()`/`waitpid()` para no dejar procesos zombie, evitando saturar el sistema.

### `simuladores/simulador_memoria.c`

Debe consumir memoria progresivamente mediante reserva dinamica (`malloc()`) en un bucle controlado.

Objetivo:

- demostrar deteccion de consumo anormal de memoria;
- activar alerta por superar el umbral configurado.

Debe liberar la memoria reservada con `free()` al finalizar el proceso.

### `simuladores/simulador_archivo.c`

Debe escribir rapidamente en un archivo temporal usando `fopen()`/`fwrite()` en un bucle.

Objetivo:

- demostrar deteccion de archivos que crecen rapido;
- activar alerta por tasa de crecimiento anormal.

Debe escribir en una carpeta controlada, por ejemplo:

```text
/tmp/anti_bunny_demo
```

## Como deben trabajar los integrantes

El equipo debe trabajar por modulos, pero integrando constantemente. Cada modulo se compone de un par `.h` (interfaz/estructuras) y `.c` (implementacion).

### Integrante 1: procesos y recursos

Responsable de:

- `monitor_procesos.h` / `monitor_procesos.c`;
- `monitor_recursos.h` / `monitor_recursos.c`;
- pruebas de procesos hijos (lectura correcta de `/proc`);
- pruebas de consumo de CPU/memoria.

Debe entregar datos en `struct` simples (`ProcesoInfo`, `RecursoInfo`) para que el motor de deteccion pueda usarlos, respetando las firmas de funciones acordadas con el equipo.

### Integrante 2: archivos y simuladores

Responsable de:

- `monitor_archivos.h` / `monitor_archivos.c`;
- `simulador_archivo.c`;
- apoyo en `simulador_fork_bomb.c`;
- pruebas de crecimiento de ficheros.

Debe garantizar que las pruebas sean controladas y no llenen el disco, y que las funciones expuestas en `monitor_archivos.h` devuelvan estructuras `ArchivoInfo` compatibles con el motor de deteccion.

### Integrante 3: deteccion, respuesta e integracion

Responsable de:

- `motor_deteccion.h` / `motor_deteccion.c`;
- `respuesta.h` / `respuesta.c`;
- `main.c`;
- `Makefile`;
- logs;
- guia de demostracion.

Debe integrar los datos de todos los modulos (mediante las estructuras y firmas de funciones definidas en los `.h`) y generar alertas claras.

## Forma de trabajo conjunta

Para evitar conflictos, cada integrante debe trabajar en su modulo y coordinar las firmas de funciones y estructuras de entrada/salida.

Reglas de trabajo:

- no cambiar nombres de archivos `.h`/`.c` ni firmas de funciones sin avisar al equipo;
- no modificar el modulo de otro integrante sin coordinar;
- mantener funciones pequenas y faciles de probar (una responsabilidad por funcion);
- registrar cualquier cambio importante en la documentacion;
- probar cada modulo individualmente (compilando con su propio `main` de prueba o mediante el `Makefile`) antes de integrarlo;
- usar issues de GitHub para dividir tareas;
- mover tarjetas del Project entre `Todo`, `In Progress` y `Done`.

## Integracion entre modulos

Los monitores no deben decidir si algo es virus. Solo recolectan datos desde `/proc` y el sistema de archivos, y los entregan en estructuras (`struct`) en memoria.

El motor de deteccion no debe leer directamente `/proc` ni los directorios. Solo analiza las estructuras de datos recibidas.

El modulo de respuesta no debe calcular metricas. Solo reporta o actua (log y, opcionalmente, `kill()`).

Flujo correcto:

```text
monitores (/proc, dirent.h, sys/stat.h) -> motor_deteccion -> respuesta -> logs
```

Todos los modulos se compilan y enlazan mediante el `Makefile` en la raiz del proyecto, que debe generar el ejecutable principal (por ejemplo, `anti_bunny_virus`) y los binarios de los simuladores.

## Demostracion final

La demostracion debe mostrar cuatro casos:

1. **Sistema normal**
   - Ejecutar el binario principal (`./anti_bunny_virus`).
   - No deben aparecer alertas importantes.

2. **Simulacion de fork bomb**
   - Ejecutar el binario `simulador_fork_bomb`.
   - Debe aparecer alerta por exceso de hijos.

3. **Simulacion de memoria**
   - Ejecutar el binario `simulador_memoria`.
   - Debe aparecer alerta por consumo excesivo.

4. **Simulacion de archivo**
   - Ejecutar el binario `simulador_archivo`.
   - Debe aparecer alerta por crecimiento acelerado de archivo.

En todos los casos, el programa principal debe poder detenerse limpiamente con `Ctrl+C` (senal `SIGINT`), cerrando el log correctamente.

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

La solucion debe mantenerse liviana: no debe escanear todo el sistema constantemente, sino monitorear por intervalos (usando `sleep()`/`nanosleep()` entre ciclos) y enfocarse en senales de comportamiento anomalo, aprovechando el bajo costo de leer `/proc` directamente en C.
