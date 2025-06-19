#include "struct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void my_strncpy(char dest[], size_t len_dest, char *src)
{
    for (size_t i = 0; i < len_dest; i++)
        dest[i] = 0;
    if (!src)
        return;
    size_t len_min = len_dest > strlen(src) ? strlen(src) : len_dest;
    if (len_min == len_dest)
        len_min--;
    dest = strncpy(dest, src, len_min);
    return;
}

char *my_strtok(char *buffer, char *delim, size_t *index, int max_size)
{
    // search for the next \n or the \0
    size_t iter = *index;
    int count = 0;
    int match = 0;
    while (buffer[iter] != '\0' && !match)
    {
        // check if we reached delim
        if (strlen(&buffer[*index]) - count > strlen(delim))
        {
            int delim_iter = 0;
            while (1)
            {
                if (delim[delim_iter] == '\0')
                {
                    match = 1;
                    iter += delim_iter - 1;
                    count += delim_iter - 1;
                    break;
                }
                if (delim[delim_iter] != buffer[iter + delim_iter])
                    break;
                delim_iter++;
            }
        }
        if (match)
            break;
        iter++;
        count++;
    }

    // delimiter is reached
    char *new_buffer = calloc(max_size, sizeof(char));
    buffer[iter] = '\0';
    for (int i = 0; i < max_size - 1 && buffer[*index + i] != '\0'; i++)
        new_buffer[i] = buffer[*index + i];
    *index = *index + count + 1;
    return new_buffer;
}

struct payload *to_payload(char *buffer)
{
    // check arg
    if (!buffer)
        return NULL;

    struct payload *payload = calloc(1, sizeof(struct payload));
    if (!payload)
        return NULL;

    size_t index = 0;
    // skip size buffer
    char *token_size = my_strtok(buffer, "\n", &index, 5);
    free(token_size);

    // status
    char *token_status = my_strtok(buffer, "\n", &index, 2);
    if (!token_status)
    {
        free(payload);
        return NULL;
    }

    payload->status = atoi(token_status);
    free(token_status);

    // command
    char *token_command = my_strtok(buffer, "\n", &index, MAX_COMMAND_SIZE);
    my_strncpy(payload->command, MAX_COMMAND_SIZE, token_command);
    free(token_command);

    // parameters
    if (strcmp(payload->command, "SEND-DM") || strcmp(payload->command, "BROADCAST"))
    {
        my_strncpy(payload->parameters, MAX_PARAMETERS_SIZE, "\n");
        index++;
    }
    else
    {
        char *token_parameters =
            my_strtok(buffer, "\n\n", &index, MAX_PARAMETERS_SIZE);
        my_strncpy(payload->parameters, MAX_PARAMETERS_SIZE, token_parameters);
        free(token_parameters);
    }

    // message
    char *token_message = my_strtok(buffer, "\0", &index, MAX_MESSAGE_SIZE);
    my_strncpy(payload->message, MAX_MESSAGE_SIZE, token_message);
    free(token_message);

    // size
    payload->size = strlen(payload->message);

    return payload;
}

char *itoa(int n)
{
    size_t len = 1;
    for (int cp_n = n; cp_n > 9; len++)
        cp_n /= 10;
    char *res = calloc(len + 1, sizeof(char));
    int i = len - 1;
    while (n >= 0 && i >= 0)
    {
        int m = n % 10;
        res[i--] = m + '0';
        n /= 10;
    }
    return res;
}

char *to_buffer(struct payload *payload)
{
    if (!payload)
        return NULL;
    char *size_itoa = itoa(payload->size);
    char *status_itoa = itoa(payload->status);
    size_t size = strlen(size_itoa) + strlen(status_itoa)
        + strlen(payload->command) + strlen(payload->parameters)
        + strlen(payload->message) + 5 + // '\n'
        1; // '\0'
    char *buffer = calloc(size, sizeof(char));
    strcat(buffer, size_itoa);
    strcat(buffer, "\n");
    strcat(buffer, status_itoa);
    strcat(buffer, "\n");
    strcat(buffer, payload->command);
    strcat(buffer, "\n");
    strcat(buffer, payload->parameters);
    strcat(buffer, "\n");
    if (buffer[strlen(buffer) - 2] != '\n')
        strcat(buffer, "\n");
    strcat(buffer, payload->message);
    free(size_itoa);
    free(status_itoa);
    return buffer;
}
