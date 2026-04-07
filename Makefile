CC = gcc
CFLAGS = -Wall

all:
	$(CC) src/server.c -o bin/server
	$(CC) src/sensor.c -o bin/sensor

clean:
	rm -f bin/server bin/sensor
