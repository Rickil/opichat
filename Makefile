CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c99 -pthread #-pedantic
LDFLAGS = -fsanitize=address -g

CFILES = src/client_function.c \
		 src/check.c \
		 src/struct.c

CCLIENT = src/basic_client.c

CSERVER = src/epoll-server.c \
		  src/utils/xalloc.c \
		  src/connection.c \
		  src/server_function.c

BIN = opichat_client \
	  opichat_server

all: opichat_client opichat_server

opichat_client: $(CFILES) $(CCLIENT)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

opichat_server: $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CSERVER) $^ -o $@

clean:
	$(RM) $(BIN)
