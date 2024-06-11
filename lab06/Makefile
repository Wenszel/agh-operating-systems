CC=gcc
CFLAGS=-Wall -lrt

all: server.out client.out

server.out: server.c
	$(CC) $(CFLAGS) server.c -o server.out

client.out: client.c
	$(CC) $(CFLAGS) client.c -o client.out

clean:
	rm -f server.out client.out
