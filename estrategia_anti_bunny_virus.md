# Estrategia anti-bunny-virus

## Que es un bunny-virus

Un **bunny-virus** es un ataque de agotamiento de recursos. Su comportamiento se asocia con los ataques tipo **rabbit/wabbit** o **fork bomb**, donde un proceso se replica o genera actividad de forma acelerada hasta consumir memoria, CPU, procesos disponibles o almacenamiento.

Cuando el sistema se queda sin recursos, deja de responder correctamente: no acepta comandos, se vuelve lento o queda paralizado. Por eso, el objetivo principal no es solo eliminar archivos despues del ataque, sino **detectar el comportamiento anomalo antes de que el sistema colapse**.

## Justificacion desde el estado del arte

Los articulos revisados muestran que este tipo de amenaza se entiende mejor como un problema de **comportamiento y agotamiento de recursos**, no como un virus que necesariamente pueda identificarse por nombre.

Los trabajos sobre **fork bomb** indican que el ataque puede detectarse observando la creacion rapida de procesos y el consumo anormal de recursos. Los trabajos sobre **resource exhaustion** refuerzan que la defensa debe anticiparse al agotamiento total, midiendo el crecimiento del consumo en ventanas de tiempo.

Por eso, la estrategia mas adecuada para este proyecto es una deteccion basada en comportamiento.

## Estrategia propuesta

El sistema anti-bunny-virus funcionara como un monitor en tiempo real. Su tarea sera observar procesos, memoria y archivos para detectar crecimiento anormal.

La estrategia se divide en cuatro partes:

1. **Monitoreo de procesos**

   Se registrara cuantos procesos crea cada proceso padre. Si un proceso empieza a generar demasiados hijos en poco tiempo, sera marcado como sospechoso.

2. **Monitoreo de memoria y CPU**

   Se medira el consumo de memoria y CPU por proceso. Un crecimiento brusco o sostenido indicara posible agotamiento de recursos.

3. **Monitoreo de archivos**

   Se observara el tamano de los ficheros y su tasa de crecimiento. Si un archivo empieza a llenarse demasiado rapido, el sistema intentara identificar que proceso lo esta modificando.

4. **Respuesta temprana**

   Cuando se detecte un comportamiento sospechoso, el sistema podra generar una alerta, registrar el evento y detener o aislar el proceso responsable antes de que el sistema quede paralizado.

## Criterio de deteccion

El sistema no dependera de firmas ni nombres especificos de virus. Se basara en indicadores como:

- cantidad de procesos creados por segundo;
- crecimiento de memoria por proceso;
- uso elevado y sostenido de CPU;
- crecimiento rapido de archivos;
- relacion entre proceso sospechoso y archivo modificado.

## Finalidad

La finalidad del proyecto es prevenir que un ataque tipo bunny-virus deje el sistema inutilizable. Para lograrlo, el programa debe actuar antes del colapso, detectando patrones de replicacion, consumo excesivo y crecimiento anormal de ficheros.
