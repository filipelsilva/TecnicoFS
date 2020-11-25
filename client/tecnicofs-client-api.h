#ifndef API_H
#define API_H

#include "../tecnicofs-api-constants.h"

int tfsCreate(char *path, char nodeType);
int tfsDelete(char *path);
int tfsLookup(char *path);
int tfsMove(char *from, char *to);
int tfsPrint(char *path);
int tfsMount(char* clientName, char* serverName);
int tfsUnmount(char* clientName);

#endif /* CLIENT_H */
