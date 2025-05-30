#include "basic_server.h"

// already include in header
//#include <netdb.h>
//#include <sys/socket.h>
//#include <sys/types.h>

//#define DEFAULT_BUFFER_SIZE 2048
#define MAX_QUEUE 1024

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_and_bind(struct addrinfo *addrinfo)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
        errx(1, "Failed to create socket");

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    int res = bind(fd, addrinfo->ai_addr, addrinfo->ai_addrlen);

    if (res < 0)
    {
        close(fd);
        errx(1, "Failed to bind the socket");
    }

    return fd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints = { 0 };
    struct addrinfo *addrinfo = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(ip, port, &hints, &addrinfo))
        errx(1, "The given SERVER_IP or SERVER_PORT is not valid");

    int fd = create_and_bind(addrinfo);
    freeaddrinfo(addrinfo);
    if (listen(fd, MAX_QUEUE) < 0)
        errx(1, "error code %d", errno);

    return fd;
}

int accept_client(int socket)
{
    int fd = accept(socket, NULL, NULL);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to accept client %i connection", socket);
        close(fd);
        return -1;
    }
    fprintf(stdout, "Client connected\n");
    if (listen(socket, MAX_QUEUE) < 0)
    {
        fprintf(stderr, "socket %d can not listen\n", errno);
        fprintf(stderr, "client %i can not be listened", fd);
        close(fd);
        return -1;
    }

    return fd;
}

void communicate(int client_socket)
{
    int flags = 0; // MSG_NOSIGNAL; //MSG_DONTWAIT
    size_t size = DEFAULT_BUFFER_SIZE;
    char buffer[DEFAULT_BUFFER_SIZE] = { '\0' };
    ssize_t res = recv(client_socket, buffer, size, flags);
    if (res < 0)
    {
        fprintf(stderr, "Failed to receive data from client %i\n",
                client_socket);
        close(client_socket);
    }

    fprintf(stdout, "Received Body: %s", buffer);

    // responding part

    int respond = send(client_socket, buffer, size, flags);
    if (respond <= 0)
    {
        // fprintf(stderr, "Client %i disconnected", client_socket);
        close(client_socket);
        return;
    }
    if (respond < 0)
        fprintf(stderr, "Message not sent\n");
}

int check_comm(int res)
{
    if (res < 0)
        return 0;

    if (res == 0)
    {
        fprintf(stdout, "Client disconnected\n");
        return 0;
    }
    return 1;
}

void communicate2(int client_socket)
{
    int flags = MSG_NOSIGNAL;

    while (1)
    {
        char *buffer = calloc(DEFAULT_BUFFER_SIZE, sizeof(char));
        ssize_t recep = recv(client_socket, buffer, DEFAULT_BUFFER_SIZE, flags);
        if (!check_comm(recep))
        {
            free(buffer);
            return;
        }
        while (buffer[strlen(buffer) - 1] != '\n')
        {
            char *tmp = realloc(buffer, strlen(buffer) + DEFAULT_BUFFER_SIZE);
            if (!tmp)
            {
                free(buffer);
                return;
            }
            buffer = tmp;
            char mini_buf[DEFAULT_BUFFER_SIZE] = { 0 };
            ssize_t recep =
                recv(client_socket, mini_buf, DEFAULT_BUFFER_SIZE, flags);
            if (!check_comm(recep))
            {
                free(buffer);
                return;
            }
            buffer = strcat(buffer, mini_buf);
        }

        fprintf(stdout, "Received body: %s", buffer);

        int respond = send(client_socket, buffer, strlen(buffer), flags);
        free(buffer);
        if (!check_comm(respond))
            return;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "Usage: ./basic_server SERVER_IP SERVER_PORT");

    int fd = prepare_socket(argv[1], argv[2]);

    int fd_current;

    while (1)
    {
        fd_current = accept_client(fd);
        if (fd_current < 0)
            return 1;
        communicate2(fd_current);
        close(fd_current);
    }

    return 0;
}
