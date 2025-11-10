CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = paging_simulator

all: $(TARGET)

$(TARGET): paging_simulator.c
	$(CC) $(CFLAGS) -o $(TARGET) paging_simulator.c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
