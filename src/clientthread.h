#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include "network.h" //TODO: Remove when integrating communication functions into broadcastagent.c
#include "user.h"

enum COMMANDS {
    kickClientCommandCode = 0,
    pauseChatCommandCode = 1,
    resumeChatCommandCode = 2,
    invalidCommandCode = 3
};

void *clientthread(void *arg);

int getStringLength(Message *buffer);

int checkClientName(char *name, int length);

void handleURM(uint8_t urmCode, User *self);

void handleAdmin(Message buffer, User *self);

void closeClient(User *user);

void printList();

#endif
