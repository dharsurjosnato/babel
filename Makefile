CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
SRC = src/main.c src/dictionary.c src/lexer.c src/parser.c src/ast.c src/interpreter.c
OBJ = $(SRC:.c=.o)
TARGET = babel

.PHONY: all clean test
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -lm -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

test: $(TARGET)
	sh ./tests/smoke.sh
	sh ./tests/control.sh
