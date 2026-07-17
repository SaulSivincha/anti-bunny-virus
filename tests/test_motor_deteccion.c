#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "motor_deteccion.h"

int main(void) {
    ProcesoInfo procesos[1] = {{ .pid = 42, .hijos = 21, .nombre = "simulador" }};
    RecursoInfo recursos[1] = {{ .pid = 43, .memoria_mb = 300.0f, .nombre = "memoria" }};
    ArchivoInfo archivos[1] = {{ .crecimiento_mb_s = 6.0f, .ruta = "/tmp/demo" }};
    Alerta alertas[3];
    int total = detectar(procesos, 1, recursos, 1, archivos, 1, alertas, 3,
                         20, 256.0f, 80.0f, 5.0f);
    assert(total == 3);
    assert(alertas[0].pid == 42);
    assert(alertas[1].pid == 43);
    assert(alertas[2].pid == -1);
    assert(strcmp(alertas[0].tipo, "procesos") == 0);
    puts("test_motor_deteccion: OK");
    return 0;
}
