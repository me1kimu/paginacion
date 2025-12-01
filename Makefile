# Makefile para el simulador de paginación

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = simulador_paginacion
OBJS = main.o memory.o

# Regla principal
all: $(TARGET)

# Compilar el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Compilar archivos objeto
main.o: main.c memory.h
	$(CC) $(CFLAGS) -c main.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Regla para debugging
debug: CFLAGS += -DDEBUG
debug: clean $(TARGET)

.PHONY: all clean run debug test
test: test_page_fault.c memory.o
	$(CC) $(CFLAGS) -o test_page_fault test_page_fault.c memory.o -lm
	./test_page_fault