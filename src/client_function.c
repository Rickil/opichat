#define _POSIX_C_SOURCE 200809L

#include "client_function.h"

#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "struct.h"

// return the username in a parameter of type User=username
char *get_username(char *parameters)
{
    char *username = strtok(parameters, "=");
    while (fnmatch("From", username, 0))
        username = strtok(NULL, "=\n");
    username = strtok(NULL, "=\n");
    return username;
}

// return 0 if need nothing
// return 1 if need only payload
// return 2 if need only parameter
// return 3 if need payload and parameter
static int what(char *command)
{
    if (!strcmp(command, "PING") || !strcmp(command, "LIST-USERS"))
        return 0;
    if (!strcmp(command, "BROADCAST") || !strcmp(command, "LOGIN"))
        return 1;
    if (!strcmp(command, "SEND-DM"))
        return 3;
    return -1;
}

struct payload *get_parameter(struct payload *payload)
{
    if (!payload)
        return NULL;
    char *buffer = calloc(MAX_PARAMETERS_SIZE, sizeof(char));
    fprintf(stdout, "Parameters:\n");
    // parameter
    while (1)
    {
        size_t false_size = 0;
        char *mini_buff = NULL;
        ssize_t parameter_size = getline(&mini_buff, &false_size, stdin);
        if (parameter_size < 0)
        {
            free(mini_buff);
            free(buffer);
            return NULL;
        }
        if (!valid_parameter(payload->command, mini_buff))
        {
            fprintf(stderr, "Invalid parameter\n");
        }
        char *no_newline = strdup(mini_buff);
        no_newline[strlen(no_newline) - 1] = '\0';
        if (!strcmp(payload->command, "SEND-DM") && !check_username(no_newline))
            fprintf(stderr, "Invalid parameter\n");
        else if (!strcmp(mini_buff, "\n"))
        {
            free(no_newline);
            free(mini_buff);
            break;
        }
        else
        {
            buffer = strcat(buffer, mini_buff);
            free(mini_buff);
            free(no_newline);
        }
    }
    my_strncpy(payload->parameters, MAX_PARAMETERS_SIZE, buffer);
    free(buffer);
    return payload;
}

struct payload *get_message(struct payload *payload)
{
    if (!payload)
        return NULL;
    char *buffer = NULL;
    // message
    fprintf(stdout, "Payload:\n");
    size_t false_size = 0;
    ssize_t payload_size = getline(&buffer, &false_size, stdin);
    if (payload_size < 0)
    {
        free(buffer);
        return NULL;
    }
    buffer[payload_size - 1] = '\0';
    my_strncpy(payload->message, MAX_MESSAGE_SIZE, buffer);
    free(buffer);
    return payload;
}

char *get_command(int *quit)
{
    char *buffer = NULL;
    // command
    while (1)
    {
        fprintf(stdout, "Command:\n");
        size_t false_size = 0;
        ssize_t command_size = getline(&buffer, &false_size, stdin);
        if (command_size < 0)
        {
            free(buffer);
            return NULL;
        }
        if (!valid_command(buffer))
        {
            fprintf(stderr, "Invalid command\n");
        }
        else
        {
            buffer[command_size - 1] = '\0';
            break;
        }
    }
    // build the payload struct
    struct payload *payload = calloc(1, sizeof(struct payload));
    my_strncpy(payload->command, MAX_COMMAND_SIZE, buffer);
    int res = what(buffer);
    free(buffer);

    if (res == -1)
    {
        *quit = 1;
        return NULL;
    }
    // parameter
    if (res / 2 >= 1)
        payload = get_parameter(payload);
    // message
    if (res % 2 == 1)
        payload = get_message(payload);
    // check payload and quit
    if (payload && !strcmp(payload->message, "/quit"))
    {
        free(payload);
        *quit = 1;
        return NULL;
    }
    if (!payload)
        *quit = 0;
    // message size
    payload->size = strlen(payload->message);
    // convert payload to buffer
    char *final_buffer = to_buffer(payload);
    free(payload);
    return final_buffer;
}
