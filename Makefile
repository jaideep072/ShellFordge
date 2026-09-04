CC = gcc

CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lreadline

TARGET = shellforge

SRC = \
	src/main.c \
	src/token.c \
	src/lexer.c \
	src/parser.c \
	src/expand.c \
	src/history.c \
	src/builtin.c \
	src/executor.c

OBJ = \
	obj/main.o \
	obj/token.o \
	obj/lexer.o \
	obj/parser.o \
	obj/expand.o \
	obj/history.o \
	obj/builtin.o \
	obj/executor.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o $(TARGET)
