#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"

void *clientthread(void *arg)
{
    User *self = (User *)arg;
    debugPrint("Client thread started.");

    //TODO: Receive messages and send them to all users, skip self
    Message msg;
    int connectionStatus;
    do {
        connectionStatus = receiveMessage(self->sock, &msg);

        broadcastMessage(self, &msg);
        //debugPrint("Connection Status: %i", connectionStatus);
    } while(connectionStatus > clientClosedConnection);

    closeConnectionToClient(self->sock);
    lockUser();
    deleteUser(self);
    unlockUser();
    debugPrint("Client thread stopping.");
    return NULL;
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
void broadcastMessage(User *self, Message *buffer)
{
    //buffer->header.length = htons(buffer->len);
    iterateOverList(self, buffer, &sendMessage);
}
void sendMessage(int fd, void * buffer)
{
   if(networkSend(fd, buffer) <= communicationError)
   {
       errorPrint("Error while sending message");
   }
}


