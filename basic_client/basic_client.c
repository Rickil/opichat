#include "basic_client.h"

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_and_connect(struct addrinfo *addrinfo)
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == -1)
    {
        freeaddrinfo(addrinfo);
        errx(1, "Failed to create socket");
    }

    if (connect(fd, addrinfo->ai_addr, addrinfo->ai_addrlen))
    {
        freeaddrinfo(addrinfo);
        close(fd);
        errx(1, "Failed to connect to server");
    }

    freeaddrinfo(addrinfo);
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

    return create_and_connect(addrinfo);
}

void communicate(int server_socket)
{
    int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
    int running = 1;
    while (running)
    {
        char *buffer = NULL;
        size_t false_size = 0;
        ssize_t size = getline(&buffer, &false_size, stdin);

        if (size < 0)
        {
            free(buffer);
            break;
        }

        if (buffer)
        {
            int res = send(server_socket, buffer, strlen(buffer), flags);
            if (res == -1)
                fprintf(stderr, "Client: Failed to send message '%s'", buffer);

            free(buffer);
            if (res == 0)
            {
                close(server_socket);
                errx(1, "Server disconnected");
            }
        }

        char buffer2[DEFAULT_BUFFER_SIZE] = { 0 };
        ssize_t res = 1;
        int first = 1;
        while (buffer2[res - 1] != '\n')
        {
            res = recv(server_socket, buffer2, DEFAULT_BUFFER_SIZE - 1, 0);
            if (res < 0)
            {
                close(server_socket);
                errx(errno, "Failed to receive data from server");
            }
            if (res == 0)
            {
                close(server_socket);
                errx(1, "The server has closed");
            }

            if (first)
            {
                printf("Server answered with: ");
                first = 0;
            }

            printf("%s", buffer2);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "Usage: ./basic_client SERVER_IP SERVER_PORT");

    int fd = prepare_socket(argv[1], argv[2]);

    communicate(fd);

    close(fd);

    return 0;
}
