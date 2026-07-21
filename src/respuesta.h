#ifndef RESPUESTA_H
#define RESPUESTA_H

#include "motor_deteccion.h"

/* Registra la ruta del log de eventos del monitor. */
void configurar_logger(const char* ruta_log);
/* Carga usuario, PGID y reglas de contención del laboratorio. */
void configurar_laboratorio(const char *usuario, int pgid_autorizado, const char *ruta_pgid, int segundos_gracia);
/* Escribe alertas y aplica el modo de respuesta configurado. */
void responder(Alerta* alertas, int n_alertas, const char* modo);

#endif
