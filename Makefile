CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinc
TARGET = .build/main
ARGS = 

.PHONY: all compile run clean

all: compile run

compile:
	@mkdir -p .build
	$(CC) $(CFLAGS) src/main.c -o $(TARGET)

run:
	@./$(TARGET) $(ARGS)

clean:
	rm -rf .build

