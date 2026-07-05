# Diseno de implementacion

## Objetivo

Implementar un sistema anti-bunny-virus que monitoree el comportamiento del sistema en tiempo real para detectar procesos que consumen recursos de forma anomala, crean demasiados procesos hijos o generan crecimiento acelerado de archivos.

El sistema no se basara en firmas de virus, sino en comportamiento.

## Arquitectura general

El proyecto se dividira en cinco modulos:

1. **Monitor de procesos**
2. **Monitor de recursos**
3. **Monitor de archivos**
4. **Motor de deteccion**
5. **Modulo de respuesta y reporte**

## Flujo del sistema

1. El sistema inicia el monitoreo.
2. Se leen periodicamente los procesos activos.
3. Se mide el consumo de CPU y memoria.
4. Se revisa la cantidad de procesos hijos por proceso padre.
5. Se observan los archivos y su tasa de crecimiento.
6. El motor de deteccion compara los valores con umbrales definidos.
7. Si se detecta comportamiento sospechoso, se registra una alerta.
8. Opcionalmente, se detiene o aisla el proceso responsable.

## Modulos

### 1. Monitor de procesos

Responsable de identificar procesos activos y relaciones padre-hijo.

Datos a obtener:

- PID del proceso;
- PPID del proceso padre;
- nombre del proceso;
- usuario propietario;
- ruta del ejecutable;
- cantidad de procesos hijos.

Utilidad:

Permite detectar comportamiento tipo fork bomb, donde un proceso empieza a crear muchos procesos hijos rapidamente.

### 2. Monitor de recursos

Responsable de medir uso de memoria y CPU.

Datos a obtener:

- porcentaje de CPU;
- memoria usada;
- variacion de memoria en el tiempo;
- tiempo de ejecucion del proceso.

Utilidad:

Permite detectar agotamiento de recursos antes de que el sistema quede paralizado.

### 3. Monitor de archivos

Responsable de vigilar archivos y directorios seleccionados.

Datos a obtener:

- ruta del archivo;
- tamano actual;
- tamano anterior;
- tasa de crecimiento;
- fecha de modificacion;
- posible proceso responsable.

Utilidad:

Permite ubicar ficheros que empiezan a crecer demasiado rapido, como indica la descripcion del proyecto.

### 4. Motor de deteccion

Responsable de decidir si un comportamiento es sospechoso.

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

### 5. Modulo de respuesta y reporte

Responsable de registrar y actuar ante una deteccion.

Acciones posibles:

- mostrar alerta;
- guardar evento en logs;
- registrar PID, nombre, consumo y archivo afectado;
- sugerir detener el proceso;
- detener el proceso si el modo automatico esta habilitado.

## Division del trabajo

### Integrante 1: monitoreo de procesos y recursos

Responsabilidades:

- implementar lectura de procesos;
- obtener PID, PPID y nombre;
- calcular procesos hijos por proceso padre;
- medir CPU y memoria;
- entregar datos al motor de deteccion.

### Integrante 2: monitoreo de archivos

Responsabilidades:

- observar directorios definidos;
- medir tamano de archivos;
- calcular tasa de crecimiento;
- detectar archivos que crecen rapidamente;
- entregar eventos al motor de deteccion.

### Integrante 3: motor de deteccion, respuesta e integracion

Responsabilidades:

- definir umbrales iniciales;
- combinar senales de procesos, recursos y archivos;
- generar alertas;
- registrar logs;
- integrar los modulos;
- preparar pruebas y demostracion.

## Estructura propuesta del proyecto

```text
anti-bunny-virus/
├── src/
│   ├── monitor_procesos.py
│   ├── monitor_recursos.py
│   ├── monitor_archivos.py
│   ├── motor_deteccion.py
│   ├── respuesta.py
│   └── main.py
├── logs/
│   └── eventos.log
├── tests/
│   └── simulador_bunny.py
├── estado_del_arte/
├── estrategia_anti_bunny_virus.md
└── diseno_implementacion.md
```

## Pruebas propuestas

### Prueba 1: crecimiento de procesos

Ejecutar un simulador controlado que cree procesos hijos de manera rapida. El sistema debe detectarlo y generar alerta.

### Prueba 2: consumo de memoria

Ejecutar un proceso que aumente su uso de memoria progresivamente. El sistema debe marcarlo como sospechoso si supera el umbral.

### Prueba 3: crecimiento de archivos

Ejecutar un script que escriba datos rapidamente en un archivo. El sistema debe detectar el archivo con mayor tasa de crecimiento.

### Prueba 4: caso normal

Ejecutar aplicaciones normales del sistema. El sistema no debe generar alertas innecesarias.

## Tecnologias sugeridas

- Lenguaje: Python
- Libreria para procesos: `psutil`
- Monitoreo de archivos: `watchdog` o revision periodica con `os.stat`
- Logs: modulo `logging`
- Sistema operativo objetivo: Linux

## Resultado esperado

Al finalizar, el sistema debe ser capaz de:

- monitorear procesos activos;
- detectar creacion anormal de procesos;
- medir consumo de memoria y CPU;
- detectar archivos que crecen rapidamente;
- generar alertas;
- registrar evidencia del comportamiento sospechoso;
- justificar la deteccion segun criterios del estado del arte.
