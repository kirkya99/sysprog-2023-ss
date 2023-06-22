#include <pthread.h>
#include <mqueue.h>
#include "broadcastagent.h"
#include "util.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
//#include "network.h"
//#include "user.h"

static mqd_t messageQueue;
static pthread_t threadId;
static int broadcastStatus;
static uint8_t priority = 0;
struct mq_attr options;
uint8_t chatStatus = running;
static sem_t sem;

static void *broadcastAgent(void *arg)
{

    Message msg;
    while(1)
    {
        //TODO: Implement thread function for the broadcast agent here!
        mq_receive(messageQueue, ((char*)&msg), options.mq_msgsize, NULL);

        broadcastMessage(NULL, &msg);

    }
    return arg;
}

int broadcastAgentInit(void)
{
    //TODO: init semaphore

    if(sem_init(&sem, pshared, value)!=0)
    {
        errorPrint("Cannot initializes semaphore");
        return -1;
    }

	//TODO: create message queue
    options.mq_flags = 0;
    options.mq_maxmsg = 10;
    options.mq_msgsize = MSG_MAX;
    //options.mq_curmsgs = 0;
    int flags = O_CREAT | O_RDWR;
    int mode = 0666;
    messageQueue = mq_open("/broadcastMSQ", flags, mode , &options);
    if(messageQueue == (mqd_t) -1)
    {
        errorPrint("Error while creating MSG-Queue");
        return messageQueue;
    }
    //TODO: start thread/
    if(pthread_create(&threadId, NULL, broadcastAgent, NULL) != 0)
    {
        errorPrint("Error while creating thread for msgQueue");
    }
	return messageQueue;
}

void broadcastAgentCleanup(void)
{
    //TODO: stop thread
    pthread_cancel(threadId);
	//TODO: destroy message queue
    mq_unlink(&messageQueue);
    sem_destroy(&sem);

}

int receiveMessage(int fd, Message * buffer)
{
    int connectionStatus = networkReceive(fd, buffer);
    if(connectionStatus <= communicationError)
    {
        errorPrint("Error while receiving message");
    }
    return connectionStatus;
}
void broadcastMessage(User *user, Message *buffer)
{

    iterateOverList(user, buffer, &sendMessage);
}
void sendMessage(int fd, void * buffer)
{
    if(networkSend(fd, buffer) <= communicationError)
    {
        errorPrint("Error while sending message");
    }
}

void sendToQueue(Message *buffer, User *user)
{
    debugPrint("Number of Messages on Queue: %i",options.mq_curmsgs);
    char *text = "Discarded your message, because the chat is paused and the send queue is full!";
    Message msg = initMessage(server2clientCode);
    if(options.mq_curmsgs > options.mq_maxmsg)
    {
        msg.body.s2c.timestamp = getTime();
        strcpy(msg.body.s2c.originalSender,'\0');
        memcpy(msg.body.s2c.text, text, strlen(text));
        setMsgLength(&msg,strlen(text));
        prepareMessage(&msg);
        sendMessage(user->sock, &msg);
    }
    else
    {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0U;
    mq_timedsend(messageQueue, (char*)buffer, sizeof(Message), 0, &ts);
    }
}

void printMSQ()
{
    debugPrint("Messages: %i", options.mq_curmsgs);
}
mqd_t getMSQ()
{
    return messageQueue;
}

void pauseChat()
{
    sem_wait(&sem);
    chatStatus = paused;
}
void resumeChat()
{
    sem_post(&sem);
    chatStatus = running;
}
uint8_t getChatStatus()
{
    return chatStatus;
}



