#include "server_function.h"

#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "struct.h"

int check_comm(int res, struct connection_t **connection, int client_fd)
{
    if (res < 0)
        return 0;

    if (res == 0)
    {
        fprintf(stdout, "Client disconnected\n");
        *connection = remove_client(*connection, client_fd);
        return 0;
    }
    return 1;
}

struct connection_t *my_send(int socket, char *buffer, int flags,
                             struct connection_t *connection)
{
    ssize_t err = send(socket, buffer, strlen(buffer), flags);
    check_comm(err, &connection, socket);

    return connection;
}

struct payload *error_command(void)
{
    struct payload *error = calloc(1, sizeof(struct payload));
    error->status = 3;
    my_strncpy(error->command, MAX_COMMAND_SIZE, "INVALID");
    // error->parameters = "";
    my_strncpy(error->message, MAX_MESSAGE_SIZE, "Bad request\n");
    error->size = strlen("Bad request\n");
    return error;
}

struct connection_t *processing(struct payload *payload, int client_socket,
                                int flags, struct connection_t *connection)
{
    if (!strcmp(payload->command, "LOGIN"))
        return login(payload, client_socket, flags, connection);

    if (!strcmp(payload->command, "LIST-USERS"))
        return list_users(payload, client_socket, flags, connection);

    if (!strcmp(payload->command, "BROADCAST"))
        return broadcast(payload, client_socket, flags, connection);

    if (!strcmp(payload->command, "SEND-DM"))
        return send_dm(payload, client_socket, flags, connection);

    if (!strcmp(payload->command, "PING"))
        return ping(payload, client_socket, flags, connection);

    //  send invalid message
    struct payload *error = error_command();
    char *buffer = to_buffer(error);
    free(error);

    connection = my_send(client_socket, buffer, flags, connection);
    free(buffer);

    return connection;
}

struct connection_t *
send_notification_broadcast(int socket, char *buffer, int flags,
                            struct connection_t *connection)
{
    struct connection_t *tmp = connection;
    while (tmp)
    {
        if (socket != tmp->client_socket)
        {
            printf("> Message Out\n");
            printf("%s\n", buffer);
            connection = my_send(tmp->client_socket, buffer, flags, connection);
        }
        tmp = tmp->next;
    }

    return connection;
}

struct connection_t *broadcast(struct payload *payload, int client_socket,
                               int flags, struct connection_t *connection)
{
    // check arg // error already treated in server.c
    if (!payload || !connection)
        return connection;

    int error = 0;
    // check valid payload
    if (strcmp(payload->command, "BROADCAST"))
        error++;
    if (payload->status != 0)
        error++;
    /*if (strcmp(payload->parameters, "\n"))
      error++;*/

    // if error send error_format payload
    if (error != 0)
    {
        struct payload *error_format = error_command();
        char *buffer = to_buffer(error_format);
        free(error_format);
        printf("> Message out:\n%s\n", buffer);
        connection = my_send(client_socket, buffer, flags, connection);
        free(buffer);
        return connection;
    }

    // send notification to all users
    struct payload *notification = calloc(1, sizeof(struct payload));
    notification->status = 2;
    my_strncpy(notification->command, MAX_COMMAND_SIZE, "BROADCAST");
    strcat(notification->parameters, "From=");
    struct connection_t *client = find_client(connection, client_socket);
    if (strcmp(client->username, "\0"))
        strcat(notification->parameters, client->username);
    else
        strcat(notification->parameters, "<Anonymous>");

    // check if need to free struct connection_t *client;
    // check if do not modify connection
    my_strncpy(notification->message, MAX_MESSAGE_SIZE, payload->message);
    notification->size = strlen(notification->message);
    char *buffer_n = to_buffer(notification);
    free(notification);
    connection =
        send_notification_broadcast(client_socket, buffer_n, flags, connection);
    free(buffer_n);

    // else send response to user
    struct payload *response = calloc(1, sizeof(struct payload));
    response->status = 1;
    my_strncpy(response->command, MAX_COMMAND_SIZE, "BROADCAST");
    response->size = strlen(response->message);

    char *buffer_r = to_buffer(response);
    free(response);
    printf("> Message Out\n");
    printf("%s", buffer_r);
    connection = my_send(client_socket, buffer_r, flags, connection);

    return connection;
}

struct connection_t *ping(struct payload *payload, int client_socket, int flags,
                          struct connection_t *connection)
{
    // check arg // error already treated in server.c
    if (!payload || !connection)
        return connection;

    int error = 0;
    if (strcmp(payload->command, "PING"))
        error++;
    if (payload->status != 0)
        error++;

    // check if error
    if (error != 0)
    {
        struct payload *error_format = error_command();
        char *buffer = to_buffer(error_format);
        free(error_format);
        printf("> Message out:\n%s\n", buffer);
        connection = my_send(client_socket, buffer, flags, connection);
        free(buffer);
        return connection;
    }

    // else send response
    struct payload *response = calloc(1, sizeof(struct payload));
    response->status = 1;
    my_strncpy(response->command, MAX_COMMAND_SIZE, "PING");
    // response->parameters = "";
    my_strncpy(response->message, MAX_MESSAGE_SIZE, "PONG\n");
    response->size = strlen(response->message);
    char *buffer_r = to_buffer(response);
    free(response);
    printf("> Message Out\n");
    printf("%s\n", buffer_r);
    connection = my_send(client_socket, buffer_r, flags, connection);

    return connection;
}

struct connection_t *login(struct payload *payload, int client_socket,
                           int flags, struct connection_t *connection)
{
    // check if fnmatch valid "User=[0-9a-zA-Z]+"
    // send("Bad username") (3)
    //
    // if correct -> send (10) (1) (LOGIN) () (Logged in)

    // check arg // error already treated in server.c
    if (!payload || !connection)
        return connection;

    int error = 0;
    if (strcmp(payload->command, "LOGIN"))
        error++;
    if (payload->status != 0)
        error++;

    // check if error
    if (error != 0)
    {
        struct payload *error_format = error_command();
        char *buffer = to_buffer(error_format);
        free(error_format);
        printf("> Message out:\n%s\n", buffer);
        connection = my_send(client_socket, buffer, flags, connection);
        free(buffer);
        return connection;
    }

    // TODO login connection (epoll-server.c: accept_client function)
    struct connection_t *client_connection =
        find_client(connection, client_socket);
    strcpy(client_connection->username, payload->message);

    // else send response
    struct payload *response = calloc(1, sizeof(struct payload));
    response->status = 1;
    my_strncpy(response->command, MAX_COMMAND_SIZE, "LOGIN");
    // response->parameters = "";
    my_strncpy(response->message, MAX_MESSAGE_SIZE, "Logged in\n");
    response->size = strlen(response->message);
    char *buffer_r = to_buffer(response);
    printf("> Message out:\n%s", buffer_r);
    free(response);
    connection = my_send(client_socket, buffer_r, flags, connection);

    return connection;
}

char *concat_user(struct connection_t *connection, char message[])
{
    struct connection_t *tmp = connection;
    while (tmp)
    {
        if (tmp->username[0])
        {
            my_strncpy(&message[strlen(message)],
                       MAX_MESSAGE_SIZE - strlen(message), tmp->username);
            my_strncpy(&message[strlen(message)],
                       MAX_MESSAGE_SIZE - strlen(message), "\n");
        }
        tmp = tmp->next;
    }

    return message;
}

struct connection_t *list_users(struct payload *payload, int client_socket,
                                int flags, struct connection_t *connection)
{
    if (!payload || !connection)
        return connection;

    int error = 0;
    if (strcmp(payload->command, "LIST-USERS"))
        error++;
    if (payload->status != 0)
        error++;

    bubble_sort(connection);

    // check if error
    if (error != 0)
    {
        struct payload *error_format = error_command();
        char *buffer = to_buffer(error_format);
        free(error_format);
        printf("> Message out:\n%s\n", buffer);
        connection = my_send(client_socket, buffer, flags, connection);
        free(buffer);
        return connection;
    }

    // else send response
    struct payload *response = calloc(1, sizeof(struct payload));
    response->status = 1;
    my_strncpy(response->command, MAX_COMMAND_SIZE, "LIST-USERS");
    // need check if need take concat_user return value
    concat_user(connection, response->message);
    response->size = strlen(response->message);
    char *buffer_r = to_buffer(response);
    free(response);
    printf("> Message out:\n%s\n", buffer_r);
    connection = my_send(client_socket, buffer_r, flags, connection);

    return connection;
}

int check_userparameters_dm(char parameters[])
{
    return !fnmatch("User=[a-zA-Z0-9][a-zA-Z0-9]*\n**", parameters, 0);
}

struct connection_t *find_username(struct connection_t *connection,
                                   char parameters[])
{
    // format username
    for (size_t i = 0; parameters[i]; i++)
    {
        if (parameters[i] == '\n')
        {
            parameters[i] = '\0';
            break;
        }
    }
    char *username = parameters + 5;

    struct connection_t *tmp = connection;
    while (tmp)
    {
        if (!strcmp(tmp->username, username))
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

struct connection_t *send_dm(struct payload *payload, int client_socket,
                             int flags, struct connection_t *connection)
{
    if (!payload || !connection)
        return connection;

    int error = 0;
    if (strcmp(payload->command, "SEND-DM"))
        error++;
    if (payload->status != 0)
        error++;

    if (!check_userparameters_dm(payload->parameters))
        error++;

    struct connection_t *user = find_username(connection, payload->parameters);

    // check if error
    if (error != 0 || !user)
    {
        struct payload *error_format = error_command();
        my_strncpy(error_format->message, MAX_MESSAGE_SIZE, "User not found\n");
        error_format->size = strlen(error_format->message);
        char *buffer = to_buffer(error_format);
        printf("> Message out:\n%s\n", buffer);
        free(error_format);
        connection = my_send(client_socket, buffer, flags, connection);
        free(buffer);
        return connection;
    }

    char parameters_copy[MAX_PARAMETERS_SIZE];
    my_strncpy(parameters_copy, MAX_PARAMETERS_SIZE, payload->parameters);

    // else send notification
    struct payload *notification = calloc(1, sizeof(struct payload));
    notification->status = 2;
    my_strncpy(notification->command, MAX_COMMAND_SIZE, "SEND-DM");
    my_strncpy(notification->parameters, MAX_PARAMETERS_SIZE,
               payload->parameters);
    strcat(notification->parameters, "\nFrom=");
    struct connection_t *client = find_client(connection, client_socket);
    if (strcmp(client->username, "\0"))
        strcat(notification->parameters, client->username);
    else
        strcat(notification->parameters, "<Anonymous>");
    my_strncpy(notification->message, MAX_PARAMETERS_SIZE, payload->message);
    notification->size = payload->size;
    char *buffer_n = to_buffer(notification);
    printf("> Message out:\n%s\n", buffer_n);
    free(notification);
    connection = my_send(user->client_socket, buffer_n, flags, connection);

    // else send response to user
    struct payload *response = calloc(1, sizeof(struct payload));
    response->status = 1;
    my_strncpy(response->command, MAX_COMMAND_SIZE, "SEND-DM");
    my_strncpy(response->parameters, MAX_PARAMETERS_SIZE, parameters_copy);
    response->size = strlen(response->message);

    char *buffer_r = to_buffer(response);
    free(response);
    printf("> Message Out\n");
    printf("%s\n", buffer_r);
    connection = my_send(client_socket, buffer_r, flags, connection);

    return connection;
}
