#ifndef CHECK_H
#define CHECK_H

#include "struct.h"

int check_username(char parameters[]);

int valid_command(char *str);

int valid_parameter(char *command, char *str);

int valid_status(int status);

int valid_payload(struct payload *payload);

#endif
