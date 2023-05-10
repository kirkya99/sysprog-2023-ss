#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include "network.h" //TODO: Remove when integrating communication functions into broadcastagent.c
#include "user.h"

void *clientthread(void *arg);

int receiveMessage(int fd, Message * buffer);
void broadcastMessage(User *self, Message *buffer);
void sendMessage(int fd, void * buffer);

#endif
