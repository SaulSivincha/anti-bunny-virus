CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2 -D_POSIX_C_SOURCE=200809L
SHELL = /bin/bash
TARGET = anti-bunny-virus
SIMULADORES = simuladores/simulador_fork_bomb simuladores/simulador_memoria simuladores/simulador_archivo simuladores/simulador_degradacion
SCRIPTS_VALIDACION = $(wildcard scripts/validacion/*.sh)

SRCS = src/main.c \
       src/monitor_procesos.c \
       src/monitor_recursos.c \
       src/monitor_archivos.c \
       src/motor_deteccion.c \
       src/respuesta.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET) $(SIMULADORES)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

simuladores/simulador_fork_bomb: simuladores/simulador_fork_bomb.c
	$(CC) $(CFLAGS) -o $@ $<

simuladores/simulador_degradacion: simuladores/simulador_degradacion.c
	$(CC) $(CFLAGS) -o $@ $<

simuladores/simulador_memoria: simuladores/simulador_memoria.c
	$(CC) $(CFLAGS) -o $@ $<

simuladores/simulador_archivo: simuladores/simulador_archivo.c
	$(CC) $(CFLAGS) -o $@ $<

tests/test_motor_deteccion: tests/test_motor_deteccion.c src/motor_deteccion.c
	$(CC) $(CFLAGS) -Isrc -o $@ tests/test_motor_deteccion.c src/motor_deteccion.c

tests/test_respuesta: tests/test_respuesta.c src/respuesta.c
	$(CC) $(CFLAGS) -Isrc -o $@ tests/test_respuesta.c src/respuesta.c

test: tests/test_motor_deteccion tests/test_respuesta
	./tests/test_motor_deteccion
	./tests/test_respuesta

check-scripts:
	bash -n $(SCRIPTS_VALIDACION)
	./scripts/validacion/verificar_politica_contencion.sh
	./scripts/validacion/validar_resultados.sh

check: all test check-scripts

help:
	@echo "Objetivos disponibles:"
	@echo "  make          Compila monitor y simuladores acotados"
	@echo "  make test     Ejecuta pruebas unitarias"
	@echo "  make check    Compila y verifica pruebas, scripts y política segura"
	@echo "  make clean    Elimina artefactos de compilación"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(SIMULADORES) tests/test_motor_deteccion tests/test_respuesta

.PHONY: all clean test check check-scripts help
