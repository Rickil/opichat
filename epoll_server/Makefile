CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 #-pedantic
LDFLAGS=-fsanitize=address -D_XOPEN_SOURCE=700

CFILES=epoll-server.c \
	   connection.c \
	   utils/xalloc.c

epoll_server:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o $@

run:
	make clean
	make
	./epoll_server 127.0.0.1 8000


debug:
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -g -o epoll_server
	gdb ./epoll_server

clean:
	$(RM) epoll_server
