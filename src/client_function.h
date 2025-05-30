#ifndef FUNCTION_H
#define FUNCTION_H

// return the username in a paramater of type "User=username"
char *get_username(char *parameters);

char *get_command(int *quit);
// getline -> 'command' -> check valid
// strcmp -> fun (login, listuser, ping, broadcast, send)
// res
//
// struct [name_funct](void)
// ping: getline -> r
// listuser: getline ->  r
// broadcast: getline -> 'payload'
// login: getline -> 'payload'
// send: getline -> 'parameter'
//       getline -> 'payload'
//
// then convert struct -> char*

#endif
