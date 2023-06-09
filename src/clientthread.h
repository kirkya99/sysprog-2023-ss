#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include "network.h" //TODO: Remove when integrating communication functions into broadcastagent.c
#include "user.h"

void *clientthread(void *arg);

int getStringLength(Message *buffer);
int checkClientName(char *name, int length);

#endif
