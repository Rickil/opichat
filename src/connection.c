#include "connection.h"

#include <err.h>
#include <stdlib.h>
#include <unistd.h>

#include "struct.h"
#include "utils/xalloc.h"

struct connection_t *add_client(struct connection_t *connection,
                                int client_socket)
{
    struct connection_t *new_connection = xmalloc(sizeof(struct connection_t));

    new_connection->client_socket = client_socket;
    for (int i = 0; i < MAX_USERNAME_SIZE; i++)
        new_connection->username[i] = '\0';
    new_connection->buffer = NULL;
    new_connection->nb_read = 0;
    new_connection->next = connection;
    struct connection_t *tmp = connection;
    int max = 0;
    while (tmp)
    {
        if (tmp->client_nth > max)
            max = tmp->client_nth;
        tmp = tmp->next;
    }
    new_connection->client_nth = max + 1;

    return new_connection;
}

struct connection_t *remove_client(struct connection_t *connection,
                                   int client_socket)
{
    if (connection && connection->client_socket == client_socket)
    {
        struct connection_t *client_connection = connection->next;
        if (close(connection->client_socket) == -1)
            errx(1, "Failed to close socket");
        free(connection->buffer);
        free(connection);
        return client_connection;
    }

    struct connection_t *tmp = connection;
    while (tmp->next)
    {
        if (tmp->next->client_socket == client_socket)
        {
            struct connection_t *client_connection = tmp->next;
            tmp->next = client_connection->next;
            if (close(client_connection->client_socket) == -1)
                errx(1, "Failed to close socket");
            free(client_connection->buffer);
            free(client_connection);
            break;
        }
        tmp = tmp->next;
    }

    return connection;
}

struct connection_t *find_client(struct connection_t *connection,
                                 int client_socket)
{
    while (connection != NULL && connection->client_socket != client_socket)
        connection = connection->next;

    return connection;
}

void swap(struct connection_t *c1, struct connection_t *c2)
{
    int client_socket1 = c1->client_socket;
    int client_nth1 = c1->client_nth;
    char username1[MAX_USERNAME_SIZE] = { 0 };
    my_strncpy(username1, MAX_USERNAME_SIZE, c1->username);
    char *buffer1 = c1->buffer;
    ssize_t nb_read1 = c1->nb_read;

    c1->client_socket = c2->client_socket;
    c1->client_nth = c2->client_nth;
    my_strncpy(c1->username, MAX_USERNAME_SIZE, c2->username);
    c1->buffer = c2->buffer;
    c1->nb_read = c2->nb_read;

    c2->client_socket = client_socket1;
    c2->client_nth = client_nth1;
    my_strncpy(c2->username, MAX_USERNAME_SIZE, username1);
    c2->buffer = buffer1;
    c2->nb_read = nb_read1;
}

void bubble_sort(struct connection_t *connection)
{
    int swapped;
    struct connection_t *ptr1;
    struct connection_t *lptr = NULL;
    if (connection == NULL)
        return;
    do
    {
        swapped = 0;
        ptr1 = connection;

        while (ptr1->next != lptr)
        {
            if (ptr1->client_nth > ptr1->next->client_nth)
            {
                swap(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}
