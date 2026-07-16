CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -D_XOPEN_SOURCE=700
TARGET = anti-bunny-virus

SRCS = src/main.c \
       src/monitor_procesos.c \
       src/monitor_recursos.c \
       src/monitor_archivos.c \
       src/motor_deteccion.c \
       src/respuesta.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean