CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99
LDFLAGS=-fsanitize=address -D_XOPEN_SOURCE=700

CFILES=basic_server.c

basic_server:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o basic_server

run:
	make clean
	make
	./basic_server 127.0.0.1 8000


debug:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -DDEBUG=1 -g -o basic_server
	gdb ./basic_server

clean:
	$(RM) basic_server
