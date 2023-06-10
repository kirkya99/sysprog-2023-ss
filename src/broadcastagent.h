#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"
#include <mqueue.h>

enum TIME
{
    sec_to_nano = 1000000000,
    duration_wait = 100000000
};

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);

int receiveMessage(int fd, Message * buffer);
void broadcastMessage(User *user, Message *buffer);
void sendMessage(int fd, void * buffer);
void sendToQueue(Message *buffer);
void printMSQ();
mqd_t getMSQ();
void fillTime(struct timespec *ts);

#endif
