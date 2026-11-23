
SRC_DIR = src
INC_DIR = include
CC = gcc
CFLAGS = -Wall -I$(INC_DIR) -Wextra -pthread

SRCS   = $(SRC_DIR)/memory.c $(SRC_DIR)/tlb.c $(SRC_DIR)/main.c
HEADER = $(INC_DIR)/vm.h
TARGET = vm


all: $(TARGET) 

$(TARGET): $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

test: $(TARGET)
	./$(TARGET) addresses.txt lru fifo
	diff -u correct_esperado.txt correct.txt

clean:
	rm -f $(TARGET) correct.txt

.PHONY: all clean test