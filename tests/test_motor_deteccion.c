#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "motor_deteccion.h"

static int ejecutar(ProcesoInfo *p, RecursoInfo *r, ArchivoInfo *a, Alerta *salida) {
    return detectar(p, 1, r, 1, a, 1, salida, 8,
                    20, 8, 2, 256.0f, 32.0f, 80.0f, 5.0f);
}

int main(void) {
    ProcesoInfo proceso = {.pid = 42, .ppid = 2, .pgid = 42, .uid = 1001,
                            .hijos = 21, .hijos_nuevos = 9, .hijos_nuevos_s = 36.0f, .nombre = "simulador"};
    RecursoInfo recurso = {.pid = 43, .memoria_mb = 300.0f, .delta_memoria_mb_s = 40.0f, .nombre = "memoria"};
    ArchivoInfo archivo = {.crecimiento_mb_s = 6.0f, .ruta = "/tmp/demo", .pid_escritor = -1, .pgid_escritor = -1};
    Alerta alertas[8];

    reiniciar_estado_deteccion();
    int primera = ejecutar(&proceso, &recurso, &archivo, alertas);
    assert(primera == 1);
    assert(strcmp(alertas[0].tipo, "procesos") == 0);
    assert(strcmp(alertas[0].severidad, "sospechosa") == 0);
    assert(alertas[0].accionable == 0);

    int segunda = ejecutar(&proceso, &recurso, &archivo, alertas);
    assert(segunda == 3);
    assert(strcmp(alertas[0].severidad, "critica") == 0);
    assert(alertas[0].accionable == 1);
    assert(strcmp(alertas[1].tipo, "recursos") == 0);
    assert(strcmp(alertas[2].tipo, "archivos") == 0);
    assert(ejecutar(&proceso, &recurso, &archivo, alertas) == 0);

    proceso.hijos = 0; proceso.hijos_nuevos = 0;
    recurso.memoria_mb = 10.0f; recurso.delta_memoria_mb_s = 0.0f;
    archivo.crecimiento_mb_s = 0.0f;
    assert(ejecutar(&proceso, &recurso, &archivo, alertas) == 0);
    puts("test_motor_deteccion: OK");
    return 0;
}
