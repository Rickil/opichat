#define _XOPEN_SOURCE 500
#include <err.h>
#include <fnmatch.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

int check_username(char parameters[])
{
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

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        printf("%s -> %i\n", argv[i], check_username(argv[i]));
    return 0;
}

/*
// itoa test
int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        char *res;
        int num = atoi(argv[i]);
        res = itoa(num);
        printf("%i -> %s\n", num, res);
        free(res);
    }

    return 0;
}*/
/*
// parser test
int main(void)
{
    char *_false_input = "12\n0\nSEND-DM\nUser=Ouioui1\nJe m'appelle Tony
PSARFDSF %$%6[]][]FD[|\n"; (void)_false_input; char *_2false_input =
"12\n0\nSEND-DM\nUser=Ouioui2\n"; (void)_2false_input;

    char *false_input = "12\n0\nSEND-DM\nUser=Ouioui3\nJe m'appelle Tony
PSARFDSF %$%6[]][]FD[|\ndfsdklfjsdfjfj\nje suis inutile\n"; (void)false_input;

    char *input = strdup(_false_input);

    struct payload *my_payload = to_payload(input);
    if (!my_payload)
        errx(1, "Failed to create payload");

    printf("Payload->size = %i\n", my_payload->size);
    printf("Payload->status = %i\n", my_payload->status);
    printf("Payload->command = %s\n", my_payload->command);
    printf("Payload->parameters = %s\n", my_payload->parameters);
    printf("Payload->message = %s\n", my_payload->message);

    char *output = to_buffer(my_payload);

    printf("output = %s\n", output);


    free(output);

    free(my_payload);

    return 0;
}*/

/*
int main(int argc, char **argv)
{
    for (int i = 2; i < argc; i++)
    {
        int res = fnmatch(argv[i], argv[1], 0);
        printf("Result fnmatch of [%s] on \"%s\" -> %i\n", argv[i], argv[1],
res);
    }

    return 0;
} */
