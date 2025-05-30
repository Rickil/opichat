#ifndef SERVER_FUNCTION_H
#define SERVER_FUNCTION_H

#include "connection.h"
#include "struct.h"

int check_comm(int res, struct connection_t **connection, int client_fd);

struct connection_t *processing(struct payload *payload, int client_socket,
                                int flags, struct connection_t *connection);

struct connection_t *my_send(int socket, char *buffer, int flags,
                             struct connection_t *connection);

struct connection_t *broadcast(struct payload *payload, int client_socket,
                               int flags, struct connection_t *connection);

struct connection_t *ping(struct payload *payload, int client_socket, int flags,
                          struct connection_t *connection);

struct connection_t *login(struct payload *payload, int client_socket,
                           int flags, struct connection_t *connection);

struct connection_t *list_users(struct payload *payload, int client_socket,
                                int flags, struct connection_t *connection);

struct connection_t *send_dm(struct payload *payload, int client_socket,
                             int flags, struct connection_t *connection);

#endif
