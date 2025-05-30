CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99
LDFLAGS=-fsanitize=address -D_XOPEN_SOURCE=700

CFILES=basic_client.c

basic_client:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o basic_client

run:
	make clean
	make
	./basic_client 127.0.0.1 8000

debug:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -g -o basic_client
	gdb ./basic_client

clean:
	$(RM) basic_client
