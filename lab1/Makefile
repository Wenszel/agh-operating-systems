CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: collatzbibl.a collatzbibl.so static_client static_client.o dynamic_client dynamic_client.o shared_client shared_client.o

collatzbibl.a: collatzbibl.o
	$(CC) $(CFLAGS) -c collatzbibl.c -o collatzbibl.o
	ar rcs collatzbibl.a collatzbibl.o

static_client.o: static_client.c
	$(CC) $(CFLAGS) -c static_client.c -o static_client.o

static_client: static_client.o collatzbibl.a
	$(CC) $(CFLAGS) static_client.o -L. -l:collatzbibl.a -o static_client 

collatzbibl.so: collatzbibl.o
	$(CC) -fPIC -shared collatzbibl.o -o collatzbibl.so

shared_client: shared_client.o collatzbibl.o
	gcc -o shared_client shared_client.c -L. -l:collatzbibl.so

dynamic_client: dynamic_client.o collatzbibl.o
	$(CC) $(CFLAGS) -o dynamic_client dynamic_client.o collatzbibl.o

dynamic_client.o: dynamic_client.c
	$(CC) $(CFLAGS) -c dynamic_client.c -o dynamic_client.o

clean:
	rm -f *.o *.a *.so dynamic_client shared_client static_client