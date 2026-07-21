# Datos del informe

Esta carpeta referencia o contiene datos procesados destinados a tablas y
gráficas. Los resultados crudos generados por las VM permanecen en
`resultados/` y no deben sustituirse por valores estimados.

## Evidencia del sistema protegido

- `anti_bunny_virus_logs.json`: copia sin modificaciones del log entregado.
- `anti_bunny_hijos_por_ciclo.csv`: hijos presentes y nuevos por proceso en
  cada ciclo de persistencia, usado en la gráfica del capítulo de resultados.

El CSV puede regenerarse con `jq` agrupando el JSON por `evidencia.ciclos` y
comprobando que `evidencia.hijos` y `evidencia.nuevos` tienen el mismo valor
para los siete PID de cada ciclo. La tabla del informe usa además el número de
PID únicos, el PGID, el usuario y la tasa máxima presentes en el archivo
original.
