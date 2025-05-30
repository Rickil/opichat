#include "basic_client.h"

#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_function.h"
#include "struct.h"

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

void *communicate_send(void *void_fd_thread)
{
    struct fd_thread *fd_thread = void_fd_thread;
    int server_socket = fd_thread->fd;
    while (fd_thread->continue_thread)
    {
        // send part
        int flags = 0; // MSG_DONTWAIT | MSG_NOSIGNAL;
        int quit = 0;
        char *buffer = NULL;
        buffer = get_command(&quit);
        while (!buffer)
        {
            if (!quit)
            {
                printf("send close socket\n");
                close(server_socket);
                fd_thread->continue_thread = 0;
                return NULL;
            }
            buffer = get_command(&quit);
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
                fd_thread->continue_thread = 0;
                errx(1, "Server disconnected");
            }
        }
    }
    return NULL;
}

void *communicate_recv(void *void_fd_thread)
{
    struct fd_thread *fd_thread = void_fd_thread;
    int server_socket = fd_thread->fd;
    int flags = 0;
    // receive part
    while (fd_thread->continue_thread)
    {
        char *buffer2 = calloc(sizeof(struct payload) + 5, sizeof(char));

        ssize_t res = 1;
        res = recv(server_socket, buffer2, DEFAULT_BUFFER_SIZE - 1, flags);
        if (res < 0)
        {
            close(server_socket);
            free(buffer2);
            fd_thread->continue_thread = 0;
            errx(errno, "Failed to receive data from server");
        }
        if (res == 0)
        {
            close(server_socket);
            free(buffer2);
            fd_thread->continue_thread = 0;
            errx(1, "The server has closed");
        }
        struct payload *payload = to_payload(buffer2);
        if (payload->size > 0 && payload->status == 1)
            fprintf(stdout, "< %s", payload->message);
        else if (payload->status == 2)
        {
            char *username = get_username(payload->parameters);
            if (!username)
                fprintf(stderr, "Invalid username !");
            else
                fprintf(stdout, "From %s: %s\n", username, payload->message);
        }
        else if (payload->status == 3)
            fprintf(stderr, "! %s", payload->message);
        free(buffer2);
        free(payload);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "Usage: /student <ip> <port>");

    int fd = prepare_socket(argv[1], argv[2]);
    int *fd2 = malloc(sizeof(int));
    *fd2 = fd;

    // if one of them stop, stop the other one

    pthread_t send_thread;
    pthread_t recv_thread;

    struct fd_thread *fd_thread_1 = calloc(1, sizeof(struct fd_thread));
    struct fd_thread *fd_thread_2 = calloc(1, sizeof(struct fd_thread));
    fd_thread_1->fd = fd;
    fd_thread_1->continue_thread = 1;
    fd_thread_2->fd = fd;
    fd_thread_2->continue_thread = 1;

    if (pthread_create(&send_thread, NULL, communicate_send, fd_thread_1))
    {
        fprintf(stderr, "Failed to create send thread\n");
    }
    if (pthread_create(&recv_thread, NULL, communicate_recv, fd_thread_2))
    {
        pthread_cancel(send_thread);
        fprintf(stderr, "Failed to create recv thread\n");
    }
    while (fd_thread_1->continue_thread && fd_thread_2->continue_thread)
        continue;
    if (!fd_thread_1->continue_thread)
        pthread_cancel(send_thread);
    if (!fd_thread_2->continue_thread)
        pthread_cancel(recv_thread);
    /*if (pthread_join(send_thread, NULL))
        errx(1, "can't join send_thread to main thread");
    if (pthread_join(recv_thread, NULL))
        errx(1, "can't join recv_thread to main thread");*/
    free(fd2);
    return 0;
}
