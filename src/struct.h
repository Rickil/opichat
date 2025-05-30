#ifndef STRUCT_H
#define STRUCT_H

#include <stddef.h>

#define MAX_COMMAND_SIZE 256
#define MAX_PARAMETERS_SIZE 4096
#define MAX_MESSAGE_SIZE 4096

struct fd_thread
{
    int fd;
    int continue_thread;
};

struct payload
{
    int size;
    int status;
    char command[MAX_COMMAND_SIZE];
    char parameters[MAX_PARAMETERS_SIZE];
    char message[MAX_MESSAGE_SIZE];
};

char *my_strtok(char *buffer, char *delim, size_t *index, int max_size);

void my_strncpy(char dest[], size_t len_dest, char *src);

struct payload *to_payload(char *buffer);

char *to_buffer(struct payload *payload);

char *itoa(int n);

#endif
