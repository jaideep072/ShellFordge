CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lreadline

TARGET = shellforge

SRC = src/main.c src/history.c
OBJ = obj/main.o obj/history.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/history.o: src/history.c
	$(CC) $(CFLAGS) -c src/history.c -o obj/history.o

clean:
	rm -f obj/*.o $(TARGET)
