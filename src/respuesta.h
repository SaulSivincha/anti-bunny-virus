#ifndef RESPUESTA_H
#define RESPUESTA_H

#include "motor_deteccion.h"

void configurar_logger(const char* ruta_log);
void responder(Alerta* alertas, int n_alertas, const char* modo);

#endif