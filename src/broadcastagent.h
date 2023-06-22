#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"
#include <mqueue.h>

enum CHAT_STATUS
{
    running = 0,
    paused = 1
};
enum SEMAPHORE
{
    pshared = 1,
    value = 0U
};

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);

int receiveMessage(int fd, Message * buffer);
void broadcastMessage(User *user, Message *buffer);
void sendMessage(int fd, void * buffer);
void sendToQueue(Message *buffer, User *user);
void printMSQ();
mqd_t getMSQ();
void pauseChat();
void resumeChat();
uint8_t getChatStatus();

#endif
