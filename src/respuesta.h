#ifndef RESPUESTA_H
#define RESPUESTA_H

#include "motor_deteccion.h"

void configurar_logger(const char* ruta_log);
void configurar_laboratorio(const char *usuario, int pgid_autorizado, const char *ruta_pgid, int segundos_gracia);
void responder(Alerta* alertas, int n_alertas, const char* modo);

#endif
