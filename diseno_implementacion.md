# Diseno de implementacion

## Objetivo

Implementar un sistema anti-bunny-virus en **Lenguaje C** que monitoree el comportamiento del sistema Linux en tiempo real para detectar procesos que consumen recursos de forma anomala, crean demasiados procesos hijos o generan crecimiento acelerado de archivos, interactuando de forma nativa con el sistema operativo mediante el sistema de archivos virtual `/proc` y llamadas estandar de C/POSIX.

El sistema no se basara en firmas de virus, sino en comportamiento.

## Arquitectura general

El proyecto se dividira en cinco modulos, cada uno implementado como un par de archivos `.h` (interfaz y estructuras) y `.c` (implementacion):

1. **Monitor de procesos**
2. **Monitor de recursos**
3. **Monitor de archivos**
4. **Motor de deteccion**
5. **Modulo de respuesta y reporte**

Todo el proyecto se compila mediante un **Makefile** ubicado en la raiz, que genera el ejecutable principal y los binarios de los simuladores.

## Flujo del sistema

1. El sistema inicia el monitoreo desde `main.c` y registra el manejador de la senal `SIGINT` para un cierre seguro.
2. Se leen periodicamente los procesos activos recorriendo `/proc/[pid]`.
3. Se mide el consumo de CPU y memoria leyendo `/proc/[pid]/stat` y `/proc/[pid]/status`.
4. Se revisa la cantidad de procesos hijos por proceso padre (agrupando por PPID).
5. Se observan los archivos y su tasa de crecimiento recorriendo directorios con `<dirent.h>` y consultando metadatos con `<sys/stat.h>`.
6. El motor de deteccion compara los valores con umbrales definidos en `config.h`.
7. Si se detecta comportamiento sospechoso, se registra una alerta en `logs/eventos.log` mediante `fopen`/`fprintf`.
8. Opcionalmente, se detiene o aisla el proceso responsable mediante `kill(pid, SIGTERM)`.
9. Al recibir `Ctrl+C` (`SIGINT`), el sistema cierra el log y libera recursos antes de terminar.

## Modulos

### 1. Monitor de procesos (`monitor_procesos.h` / `monitor_procesos.c`)

Responsable de identificar procesos activos y relaciones padre-hijo, leyendo directamente el pseudo-sistema de archivos `/proc`.

Datos a obtener:

- PID del proceso (nombre numerico del directorio en `/proc`);
- PPID del proceso padre (campo dentro de `/proc/[pid]/stat`);
- nombre del proceso (`/proc/[pid]/comm`);
- usuario propietario (`stat()` + `getpwuid()` sobre `/proc/[pid]`);
- ruta del ejecutable (`readlink()` sobre `/proc/[pid]/exe`);
- cantidad de procesos hijos (conteo por PPID sobre el total de procesos leidos).

Utilidad:

Permite detectar comportamiento tipo fork bomb, donde un proceso empieza a crear muchos procesos hijos rapidamente.

Firma de funcion propuesta:

```c
int leer_procesos(ProcesoInfo *lista, int max_procesos);
```

### 2. Monitor de recursos (`monitor_recursos.h` / `monitor_recursos.c`)

Responsable de medir uso de memoria y CPU a partir de archivos del sistema `/proc`.

Datos a obtener:

- porcentaje de CPU (calculado con los campos `utime`/`stime` de `/proc/[pid]/stat` entre dos lecturas separadas por el intervalo de monitoreo);
- memoria usada (`VmRSS` en `/proc/[pid]/status`);
- variacion de memoria en el tiempo (diferencia entre lecturas sucesivas);
- tiempo de ejecucion del proceso (campo `starttime` en `/proc/[pid]/stat` combinado con el uptime del sistema en `/proc/uptime`).

Utilidad:

Permite detectar agotamiento de recursos antes de que el sistema quede paralizado.

Firma de funcion propuesta:

```c
int leer_recursos(pid_t pid, RecursoInfo *info);
```

### 3. Monitor de archivos (`monitor_archivos.h` / `monitor_archivos.c`)

Responsable de vigilar archivos y directorios seleccionados, usando `<dirent.h>` para recorrerlos y `<sys/stat.h>` para obtener sus metadatos.

Datos a obtener:

- ruta del archivo (construida al recorrer el directorio con `opendir()`/`readdir()`/`closedir()`);
- tamano actual (`st_size` de `struct stat`, via `stat()`);
- tamano anterior (mantenido en memoria entre ciclos);
- tasa de crecimiento (diferencia de tamano entre ciclos dividida entre el intervalo);
- fecha de modificacion (`st_mtime` de `struct stat`);
- posible proceso responsable (cruzado opcionalmente con la lista de procesos que tengan el archivo abierto, via `/proc/[pid]/fd`).

Utilidad:

Permite ubicar ficheros que empiezan a crecer demasiado rapido, como indica la descripcion del proyecto.

Firma de funcion propuesta:

```c
int escanear_directorio(const char *ruta, ArchivoInfo *lista, int max_archivos);
```

### 4. Motor de deteccion (`motor_deteccion.h` / `motor_deteccion.c`)

Responsable de decidir si un comportamiento es sospechoso, a partir de las estructuras entregadas por los monitores.

Criterios iniciales:

- demasiados procesos hijos en poco tiempo;
- crecimiento brusco de memoria;
- CPU alta durante varios ciclos;
- archivo con crecimiento acelerado;
- combinacion de dos o mas senales anormales.

Ejemplo de regla:

```text
Si un proceso crea mas de N procesos hijos en T segundos
y ademas aumenta rapidamente su consumo de memoria,
entonces se marca como sospechoso.
```

Firma de funcion propuesta:

```c
int evaluar_alertas(ProcesoInfo *procesos, RecursoInfo *recursos,
                     ArchivoInfo *archivos, AlertaInfo *alertas, int max_alertas);
```

### 5. Modulo de respuesta y reporte (`respuesta.h` / `respuesta.c`)

Responsable de registrar y actuar ante una deteccion, usando funciones estandar de C.

Acciones posibles:

- mostrar alerta (`printf`/`fprintf` sobre `stdout`/`stderr`);
- guardar evento en logs (`fopen(..., "a")`, `fprintf()`, `fclose()`);
- registrar PID, nombre, consumo y archivo afectado, junto con marca de tiempo (`time()`/`strftime()`);
- sugerir detener el proceso;
- detener el proceso si el modo automatico esta habilitado, mediante `kill(pid, SIGTERM)`.

Firma de funcion propuesta:

```c
void registrar_alerta(const AlertaInfo *alerta, FILE *log);
```

## Division del trabajo

### Integrante 1: monitoreo de procesos y recursos

Responsabilidades:

- implementar `monitor_procesos.h`/`.c` para la lectura de procesos desde `/proc`;
- obtener PID, PPID y nombre a partir de `/proc/[pid]/stat` y `/proc/[pid]/comm`;
- calcular procesos hijos por proceso padre;
- implementar `monitor_recursos.h`/`.c` para medir CPU y memoria desde `/proc/[pid]/stat` y `/proc/[pid]/status`;
- entregar datos al motor de deteccion mediante las estructuras (`ProcesoInfo`, `RecursoInfo`) y firmas de funcion acordadas.

### Integrante 2: monitoreo de archivos

Responsabilidades:

- implementar `monitor_archivos.h`/`.c` usando `<dirent.h>` y `<sys/stat.h>`;
- observar directorios definidos en `config.h`;
- medir tamano de archivos y calcular tasa de crecimiento;
- detectar archivos que crecen rapidamente;
- entregar eventos (`ArchivoInfo`) al motor de deteccion segun la firma de funcion acordada.

### Integrante 3: motor de deteccion, respuesta e integracion

Responsabilidades:

- definir umbrales iniciales en `config.h`;
- combinar senales de procesos, recursos y archivos en `motor_deteccion.c`;
- generar alertas (`AlertaInfo`);
- registrar logs mediante `respuesta.c`;
- integrar los modulos en `main.c` y mantener el `Makefile`;
- implementar el manejo de `SIGINT` para cierre seguro;
- preparar pruebas y demostracion.

## Estructura propuesta del proyecto

```text
anti-bunny-virus/
├── src/
│   ├── monitor_procesos.h
│   ├── monitor_procesos.c
│   ├── monitor_recursos.h
│   ├── monitor_recursos.c
│   ├── monitor_archivos.h
│   ├── monitor_archivos.c
│   ├── motor_deteccion.h
│   ├── motor_deteccion.c
│   ├── respuesta.h
│   ├── respuesta.c
│   ├── config.h
│   └── main.c
├── logs/
│   └── eventos.log
├── simuladores/
│   ├── simulador_fork_bomb.c
│   ├── simulador_memoria.c
│   └── simulador_archivo.c
├── tests/
│   └── test_motor_deteccion.c
├── estado_del_arte/
├── Makefile
├── estrategia_anti_bunny_virus.md
└── diseno_implementacion.md
```

## Pruebas propuestas

### Prueba 1: crecimiento de procesos

Ejecutar un simulador controlado en C (basado en `fork()` en bucle acotado) que cree procesos hijos de manera rapida. El sistema debe detectarlo y generar alerta.

### Prueba 2: consumo de memoria

Ejecutar un proceso en C que aumente su uso de memoria progresivamente mediante reservas sucesivas con `malloc()`. El sistema debe marcarlo como sospechoso si supera el umbral.

### Prueba 3: crecimiento de archivos

Ejecutar un programa en C que escriba datos rapidamente en un archivo usando `fwrite()`. El sistema debe detectar el archivo con mayor tasa de crecimiento.

### Prueba 4: caso normal

Ejecutar aplicaciones normales del sistema. El sistema no debe generar alertas innecesarias.

## Tecnologias sugeridas

- Lenguaje: **C (estandar C11)**
- Interaccion con procesos y recursos: lectura directa del sistema de archivos virtual **`/proc`** (`/proc/[pid]/stat`, `/proc/[pid]/status`, `/proc/[pid]/comm`, `/proc/[pid]/exe`)
- Monitoreo de archivos: `<dirent.h>` para recorrer directorios y `<sys/stat.h>` para obtener metadatos de tamano
- Manejo de senales: `<signal.h>` para capturar `SIGINT` de forma segura
- Logs: funciones nativas `<stdio.h>` (`fopen`, `fprintf`, `fclose`) junto con `<time.h>` para las marcas de tiempo
- Automatizacion de compilacion: **Makefile**
- Sistema operativo objetivo: Linux

## Resultado esperado

Al finalizar, el sistema debe ser capaz de:

- monitorear procesos activos leyendo `/proc` de forma nativa;
- detectar creacion anormal de procesos;
- medir consumo de memoria y CPU a partir de archivos de `/proc`;
- detectar archivos que crecen rapidamente usando `<dirent.h>` y `<sys/stat.h>`;
- generar alertas y responder de forma segura ante `SIGINT`;
- registrar evidencia del comportamiento sospechoso en `logs/eventos.log`;
- justificar la deteccion segun criterios del estado del arte;
- compilarse de forma reproducible mediante el `Makefile` de la raiz del proyecto.
