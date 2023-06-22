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
    debugPrint("User added:");
    printList();

    //TODO: Send UserAdded of all previous registered clients to new client
    User *user = getFirstUser();
    while(user->next != NULL)
    {
        if(user != self)
        {
            strLength = strlen(&user->name);
            memcpy(uad.body.uad.name, &user->name, strLength);
            setMsgLength(&uad, strLength);
            uad.body.uad.timestamp = 0;
            prepareMessage(&uad);
            sendMessage(self->sock, &uad);
        }
        user = user->next;
    }
    unlockUser();
    //TODO: Send and receive messages
    Message c2s, s2c;
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
            if(c2s.body.c2s.text[0]=='/')
            {
                handleAdmin(c2s, self);
            }
            else
            {
            strLength = getStringLength(&c2s);
            memcpy(s2c.body.s2c.text, c2s.body.c2s.text, strLength);
            strcpy(s2c.body.s2c.originalSender, &self->name);
            s2c.body.s2c.timestamp = getTime();
            setMsgLength(&s2c, strLength);

            //TODO: Send Server2Client

            prepareMessage(&s2c);
            sendToQueue(&s2c, self);
            }
        }
    }
    handleURM(urmCode, self);



    exit:
    unlockUser();


    closeClient(self);
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
        if(strcmp(name, user->name)==0)
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

void handleURM(uint8_t urmCode, User *self)
{
    Message urm;
    uint16_t strLength;
    lockUser();
    urm = initMessage(userRemovedCode);
    urm.body.urm.code = urmCode;
    urm.body.urm.timestamp = getTime();
    strLength = strlen(self->name);
    memcpy(urm.body.urm.name, self->name, strLength);
    setMsgLength(&urm, strLength);
    if(getFirstUser()->next != NULL) {
        prepareMessage(&urm);
        broadcastMessage(self, &urm);
    }
    unlockUser();
}

void handleAdmin(Message buffer, User *self)
{
    Message s2c = initMessage(server2clientCode);
    char text [TEXT_MAX];

    char *command;
    uint16_t strLength = getStringLength(&buffer);
    memcpy(text, buffer.body.c2s.text, strLength);
    uint8_t commandCode;
    int ret = 0;
    uint16_t length;

    //TODO: Identify the send command
    if(strncmp(text, "/kick", 5) == 0 && strLength > 6)
    {
        commandCode = kickClientCommandCode;
    }
    else if(strncmp(text, "/pause", 6) == 0)
    {
        commandCode = pauseChatCommandCode;
    }
    else if(strncmp(text, "/resume", 7) == 0)
    {
        commandCode = resumeChatCommandCode;
    }
    else
    {
        commandCode = invalidCommandCode;
    }

    //TODO: Check the command
    if(commandCode == invalidCommandCode)
    {
        strcpy(text, "Invalid Command!");
    }
    else if(strcmp(self->name, "Admin") != 0)
    {
        switch (commandCode) {
            case kickClientCommandCode:
                strcpy(text, "You must be administrator to use the /kick Command!");
                break;
            case pauseChatCommandCode:
                strcpy(text, "You must be administrator to use the /pause Command!");
                break;
            case resumeChatCommandCode:
                strcpy(text, "You must be administrator to use the /resume Command!");
                break;
        }
    }
    else {
        if (commandCode == kickClientCommandCode) {
            char tbkName[NAME_MAX] = "";
            uint16_t length = getStringLength(&buffer);
            uint16_t nameLength = length - 6;
            memcpy(tbkName, buffer.body.c2s.text + 6, length);
            tbkName[length] = '\0';
            User *it = getFirstUser();
            while(it != NULL && strcmp(tbkName, it->name)==0)
            {
                it = it->next;
            }
            User *tbkUser = it;
            if (tbkUser == NULL) {
                strcpy(text, "User to /kick does not exist on the server");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
            } else
            {
                Message urm = initMessage(userRemovedCode);
                urm.body.urm.code = kickedFromTheServerCode;
                memcpy(urm.body.urm.name, tbkName, nameLength);
                urm.body.urm.timestamp = getTime();
                setMsgLength(&urm, nameLength);
                prepareMessage(&urm);
                broadcastMessage(tbkUser, &urm);
                closeClient(tbkUser);

            }
        }
        if (commandCode == pauseChatCommandCode) {
            if (getChatStatus() == paused) {
                strcpy(text, "The chat is already paused!");
            } else {
                strcpy(text, "Chat paused by administrator.");
                pauseChat();
            }
        }
        if (commandCode == resumeChatCommandCode) {
            if (getChatStatus() == running) {
                strcpy(text, "The chat is not paused!");
            } else {
                strcpy(text, "The chat is no longer paused.");
                resumeChat();
            }
        }
    }
    if(commandCode != kickClientCommandCode)
    {
        strLength = strlen(text);
        s2c.body.s2c.timestamp = getTime();
        s2c.body.s2c.originalSender[0] = '\0';
        memcpy(s2c.body.s2c.text, text, strLength);
        setMsgLength(&s2c, strLength);
        prepareMessage(&s2c);
        sendMessage(self->sock, &s2c);
    }
}

void closeClient(User *user)
{
    lockUser();
    deleteUser(user);
    unlockUser();
    debugPrint("User removed:");
    printList();
    pthread_cancel(user->thread);
    close(user->sock);
    free(user);
}

void printList()
{
    User *it = getFirstUser();
    while(it != NULL)
    {
        debugPrint(it->name);
        it = it->next;
    }
}


