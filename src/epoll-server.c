#include "epoll-server.h"

#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "connection.h"
#include "server_function.h"
#include "struct.h"

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

/**
 * \brief Accept a new client and add it to the connection_t struct
 *
 * \param epoll_instance: the epoll instance
 * \param server_socket: listening socket
 * \param connection: the connection linked list with all the current
 * connections
 *
 * \return The connection struct with the new client added
 */
struct connection_t *accept_client(int epoll_instance, int server_socket,
                                   struct connection_t *connection)
{
    int fd_client = accept(server_socket, NULL, NULL);
    if (fd_client < 0)
    {
        fprintf(stderr, "Failed to accept client connection");
        return connection;
    }
    fprintf(stdout, "Client connected\n");
    if (listen(server_socket, MAX_QUEUE))
    {
        close(fd_client);
        fprintf(stderr, "Failed to listen, max clients reached\n");
        return connection;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN; // | EPOLLOUT;
    ev.data.fd = fd_client;

    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd_client, &ev) < 0)
    {
        close(fd_client);
        fprintf(stderr, "Failed to add client socket into epoll instance");
        return connection;
    }

    return add_client(connection, fd_client);
}

struct connection_t *old_communicate(int client_socket,
                                     struct connection_t *connection)
{
    int flags = MSG_NOSIGNAL;

    char *buffer = calloc(sizeof(struct payload) + 5, sizeof(char));
    ssize_t recep =
        recv(client_socket, buffer, sizeof(struct payload) + 5, flags);
    if (!check_comm(recep, &connection, client_socket))
        return connection;
    return connection;
}

// new communicate function
struct connection_t *communicate(int client_socket,
                                 struct connection_t *connection)
{
    int flags = MSG_NOSIGNAL;

    char *buffer = calloc(sizeof(struct payload) + 5, sizeof(char));

    ssize_t recep_size =
        recv(client_socket, buffer, sizeof(struct payload) + 5, flags);

    if (!check_comm(recep_size, &connection, client_socket))
        return connection;

    printf("Client data\n< Request in:\n%s\n", buffer);

    struct payload *payload = to_payload(buffer);
    free(buffer);

    connection = processing(payload, client_socket, flags, connection);
    free(payload);

    return connection;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "Usage: ./epoll-server SERVER_IP SERVER_PORT");

    int fd = prepare_socket(argv[1], argv[2]);

    int epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        close(fd);
        errx(1, "Failed to create epoll");
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    struct connection_t *res = NULL;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        close(fd);
        close(epollfd);
        errx(1, "Failed to add server socket into epoll instance");
    }

    while (1)
    {
        struct epoll_event events[MAX_EVENTS] = { 0 };
        int events_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        for (int event_idx = 0; event_idx < events_count; event_idx++)
        {
            int sock = events[event_idx].data.fd;
            struct connection_t *client = find_client(res, sock);
            if (!client)
                res = accept_client(epollfd, fd, res);
            else
            {
                // handle communication
                res = communicate(sock, res);
            }
        }
    }

    close(fd);

    return 0;
}
