#include "check.h"

#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>

// need to check if syntax valid
// if valid, apply to other functions to gain space
// if not, do the classic syntax
int valid_command(char *str)
{
    int res = 0;
    res |= !strcmp(str, "PING\n");
    res |= !strcmp(str, "LIST-USERS\n");
    res |= !strcmp(str, "LOGIN\n");
    res |= !strcmp(str, "SEND-DM\n");
    res |= !strcmp(str, "BROADCAST\n");
    return res;
}

int check_username(char parameters[])
{
    if (!strcmp(parameters, ""))
        return 1;
    size_t i = 0;
    char *user_format = "User=";
    size_t format_len = strlen(user_format);

    while (parameters[i])
    {
        if (i < format_len)
        {
            if (user_format[i] != parameters[i])
                return 0;
        }
        else
        {
            char *test = malloc(sizeof(char));
            test[0] = parameters[i];
            int res = fnmatch("[a-zA-Z0-9]", test, 0);
            free(test);
            if (res)
                return 0;
        }
        i++;
    }

    return i == strlen(parameters) && i > format_len;
}

int valid_parameter(char *command, char *str)
{
    // no parameter check
    if (!strcmp(command, "PING") || !strcmp(command, "LIST-USERS")
        || !strcmp(command, "LOGIN"))
    {
        if (!strcmp(str, ""))
            return 1;
    }

    // specific parameter check
    if (!strcmp(command, "SEND-DM"))
    {
        // 0-9 a-z A-Z (min 1 time)
        // later, need to check if valid +[...]
        if (fnmatch("User=*", str, 0) == 0)
            return 1;
        if (!strcmp(str, "\n"))
            return 1;
    }

    if (!strcmp(command, "BROADCAST"))
    {
        // same as SEND-DM
        if (!strcmp(str, "\n") || !fnmatch("From=*", str, 0))
            return 1;
    }

    return 0;
}

int valid_status(int status)
{
    return 0 <= status && status <= 3;
}

int valid_payload(struct payload *payload)
{
    if (!payload)
        return 0;

    size_t res = 0;
    res += valid_command(payload->command);
    res += valid_parameter(payload->command, payload->parameters);
    res += valid_status(payload->status);
    int strlen_message = strlen(payload->message);
    res += strlen_message == payload->size;

    return res == 4;
}
