# Utilidades de validación

Estos scripts preparan evidencia para la issue #6. No ejecutan simuladores ni
activan contención; los escenarios se ejecutarán únicamente en la VM de
laboratorio.

## Flujo previsto

1. Ejecutar `./scripts/validacion/verificar_entorno.sh`.
2. Crear una carpeta de corrida, por ejemplo:

   ```bash
   ./scripts/validacion/preparar_corrida.sh procesos 1
   ```

3. Anotar en `entorno.md` los datos reales de la VM y del snapshot.
4. Ejecutar el escenario acordado en la VM, una sola vez.
5. Ejecutar `recolectar_evidencia.sh` con la ruta que imprimió el paso 2.
6. Completar `metricas.csv` y `resumen.md` con datos observados, no estimados.

La carpeta `resultados/` está ignorada por Git: contiene evidencia generada en
cada ambiente y no forma parte del código fuente.
