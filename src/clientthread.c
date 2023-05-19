#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"

void *clientthread(void *arg)
{
    User *self = (User *)arg;
    debugPrint("Client thread started.");
    int connectionStatus;

    //TODO: Receive LoginRequest
    lockUser();
    Message *lrq;
    connectionStatus = receiveMessage(self->sock, &lrq);
    if(connectionStatus <= 0)
    {
        goto exit;
    }
    int strLength = getStringLength(&lrq);
    char *name;
    memcpy(name, lrq->body.lrq.name, strLength);
    //TODO: Send LoginResponse
    Message *lre = initMessage(loginResponseCode);
    lre->body.lre.code = checkClientName(name, strLength);
    char *sName = "reference-server";
    memcpy(lre->body.lre.sName, sName, strlen(sName));

    setMsgLength(lre, strlen(sName));
    prepareMessage(lre);
    sendMessage(self->sock,lre);
    if(lre->body.lre.code != 0)
    {
        goto exit;
    }
    memcpy(self->name, lrq->body.lrq.name, strLength);
    self->name[strLength] = '\0';

    //TODO: Send UserAdded to all clients
    Message *uad = initMessage(userAddedCode);
    memcpy(uad->body.uad.name, lrq->body.lrq.name, strLength);
    setMsgLength(uad, strLength);
    uad->body.uad.timestamp = time(NULL);
    prepareMessage(uad);
    sendMessage(self->sock, uad);
    broadcastMessage(self, uad);
    unlockUser();

    //TODO: Send and receive messages
    Message *c2s, *s2c, *urm;
    char *text;
    uint8_t urmCode;
    int loop = 1;
    while(loop == 1)
    {
        //TODO: Receive Client2Server
        connectionStatus = receiveMessage(self->sock, c2s);
        if(connectionStatus == clientClosedConnection)
        {
            urmCode = connectionClosedByClientCode;
            loop = 0;
        }
        if(connectionStatus == communicationError)
        {
            urmCode = communicationErrorCode;
            loop = 0;
        }
        strLength = getStringLength(c2s);
        memcpy(s2c->body.s2c.text, c2s->body.c2s.text, strLength);
        strcpy(s2c->body.s2c.originalSender, self->name);
        s2c->body.s2c.timestamp = time(NULL);
        setMsgLength(s2c, strLength);

        //TODO: Send Server2Client
        prepareMessage(s2c);
        broadcastMessage(self, s2c);
    }
    lockUser();
    urm = removeUser(urmCode, self->name, strlen(self->name));
    if(getFirstUser() != NULL) {
        prepareMessage(urm);
        sendMessage(self->sock, urm);
        broadcastMessage(self, urm);
    }
    unlockUser();

    exit:
    unlockUser();
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
int getStringLength(Message *buffer)
{
    int length = 0;
    switch (buffer->header.type) {
        case loginRequestCode:
            length = buffer->header.length - sizeof(buffer->body.lrq.version) - sizeof(buffer->body.lrq.magic);
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
        if(strcmp(name, user)==0)
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

Message *removeUser(uint8_t code, char *userName, uint16_t nameLenght)
{
    Message *urm = initMessage(5);
    urm->body.urm.code = code;
    urm->body.urm.timestamp = time(NULL);
    memcpy(urm->body.urm.name, userName, nameLenght);
    setMsgLength(urm->body.urm.name, nameLenght);
    return urm;
}



