/*
 * Interfaz del módulo de respuesta. Configura el log y el grupo autorizado, y
 * procesa las alertas en modo de registro o contención de laboratorio.
 */
#ifndef RESPUESTA_H
#define RESPUESTA_H

#include "motor_deteccion.h"

// Selecciona la ruta donde se anexarán los eventos.
void configurar_logger(const char* ruta_log);
// Restringe la contención al usuario y PGID autorizados para la prueba.
void configurar_laboratorio(const char *usuario, int pgid_autorizado, const char *ruta_pgid, int segundos_gracia);
// Registra todas las alertas y actúa solo si el modo y la política lo permiten.
void responder(Alerta* alertas, int n_alertas, const char* modo);

#endif
