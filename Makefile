CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
TARGET = paging_simulator
ALT_TARGET = simulador_paginacion
OBJS = main.o memory.o

all: $(TARGET)

$(TARGET): paging_simulator.c
	$(CC) $(CFLAGS) -o $(TARGET) paging_simulator.c

$(ALT_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(ALT_TARGET) $(OBJS) -lm

main.o: main.c memory.h
	$(CC) $(CFLAGS) -c main.c

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c

test_page_fault: test_page_fault.c memory.o
	$(CC) $(CFLAGS) -o test_page_fault test_page_fault.c memory.o -lm

test: test_simulator.sh
	./test_simulator.sh

testfault: test_page_fault
	./test_page_fault

clean:
	rm -f $(TARGET) $(ALT_TARGET) $(OBJS) test_page_fault

run: $(TARGET)
	./$(TARGET)

run-alt: $(ALT_TARGET)
	./$(ALT_TARGET)

.PHONY: all clean run run-alt test testfault
