#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include "broadcastagent.h"
#include <mqueue.h>

void *clientthread(void *arg)
{
    User *self = (User *)arg;
    debugPrint("Client thread started.");
    int connectionStatus;

    //TODO: Receive LoginRequest
    lockUser();
    Message lrq;
    connectionStatus = receiveMessage(self->sock, &lrq);
    if(connectionStatus <= 0)
    {
        goto exit;
    }
    int strLength = getStringLength(&lrq);
    char name [NAME_MAX];
    memcpy(name, lrq.body.lrq.name, strLength);
    //TODO: Send LoginResponse
    Message lre = initMessage(loginResponseCode);
    lre.body.lre.code = checkClientName(name, strLength);
    char *sName = "reference-server";
    int sNameLen = strlen(sName);
    memcpy(lre.body.lre.sName, sName, sNameLen);

    setMsgLength(&lre, strlen(sName));
    prepareMessage(&lre);
    sendMessage(self->sock,&lre);
    if(lre.body.lre.code != 0)
    {
        goto exit;
    }
    name[strLength] = '\0';
    strcpy(&self->name, name);

    //TODO: Send UserAdded to all clients
    Message uad = initMessage(userAddedCode);
    memcpy(uad.body.uad.name, lrq.body.lrq.name, strLength);
    setMsgLength(&uad, strLength);
    uad.body.uad.timestamp = time(NULL);
    prepareMessage(&uad);
    broadcastMessage(NULL, &uad);

    //TODO: Send UserAdded of all previous registered clients to new client
    User *user = getFirstUser();
    while(user->next != NULL)
    {
        if(user != self)
        {
            strLength = strlen(&user->name);
            memcpy(uad.body.uad.name, &user->name, strLength);
            setMsgLength(&uad, strLength);
            uad.body.uad.timestamp = getTime();
            prepareMessage(&uad);
            sendMessage(self->sock, &uad);
        }
        user = user->next;
    }
    unlockUser();
    //printUserList();
    //TODO: Send and receive messages
    Message c2s, s2c, urm;
    char text [TEXT_MAX];
    uint8_t urmCode;
    int loop = 1;

    while(loop == 1) {
        int test = 1;

        //TODO: Receive Client2Server
        s2c = initMessage(server2clientCode);
        connectionStatus = receiveMessage(self->sock, &c2s);
        if (connectionStatus == clientClosedConnection) {
            urmCode = connectionClosedByClientCode;
            loop = 0;
        }
        if (connectionStatus == communicationError) {
            urmCode = communicationErrorCode;
            loop = 0;
        }
        if (connectionStatus > clientClosedConnection) {
            strLength = getStringLength(&c2s);
            memcpy(s2c.body.s2c.text, c2s.body.c2s.text, strLength);
            strcpy(s2c.body.s2c.originalSender, &self->name);
            s2c.body.s2c.timestamp = getTime();
            setMsgLength(&s2c, strLength);

            //TODO: Send Server2Client

            prepareMessage(&s2c);
            sendToQueue(&s2c);
        }
    }
    lockUser();
    urm = initMessage(userRemovedCode);
    urm.body.urm.code = urmCode;
    urm.body.urm.timestamp = getTime();
    strLength = strlen(&self->name);
    if(self->name != NULL)
    {
        memcpy(urm.body.urm.name, &self->name, strLength);
    }
    setMsgLength(&urm, strLength);
    if(getFirstUser()->next != NULL) {
        prepareMessage(&urm);
        broadcastMessage(self, &urm);
    }
    unlockUser();

    exit:
    unlockUser();
    closeConnectionToClient(self->sock);
    lockUser();
    deleteUser(self);
    self = NULL;
    //free(self);
    unlockUser();
    //printUserList();
    debugPrint("Client thread stopping.");
    return NULL;
}

int getStringLength(Message *buffer)
{
    int length = 0;
    switch (buffer->header.type) {
        case loginRequestCode:
            length = buffer->header.length - sizeof(buffer->body.lrq.version) - sizeof(buffer->body.lrq.magic);
            infoPrint("%i", length);
            break;
        case client2ServerCode:
            length = buffer->header.length;
            break;
    }
    return length;
}
int checkClientName(char *name, int length)
{
    int statusCode = otherServerErrorCode;
    for(int i = 0; i < length; i++)
    {
        if(name[i]<33 || name[i]>=126 || name[i]==34 || name[i]==37 || name[i]==96)
        {
            statusCode = nameInvalidCode;
            return statusCode;
        }
        else {
            statusCode = successCode;
        }
    }
    User *user = getFirstUser();
    while(user != NULL)
    {
        if(strcmp(name, &user->name)==0)
        {
            statusCode = nameAlreadyTakenByAnotherUserCode;
            return statusCode;
        }
        else
        {
            statusCode = successCode;
        }
        user = user->next;
    }
    return statusCode;
}



