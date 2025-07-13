CC = gcc
CFLASS = -Wall -g
TARGET = testlib
OBJS = main.o list.o hash.o bitmap.o debug.o hex_dump.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

bitmap.o: bitmap.c bitmap.h limits.h round.h
	$(CC) $(CFLAGS) -c bitmap.c

hex_dump.o: hex_dump.c hex_dump.h
	$(CC) $(CFLAGS) -c hex_dump.c

debug.o: debug.c debug.h
	$(CC) $(CFLAGS) -c debug.c

clean:
	rm -f *.o $(TARGET)
