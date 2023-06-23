#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include "broadcastagent.h"


void *clientthread(void *arg) {
    User *self = (User *) arg;
    debugPrint("Client thread started.");
    int connectionStatus;

    //TODO: Receive LoginRequest
    Message lrq;
    connectionStatus = receiveMessage(self->sock, &lrq);
    if (connectionStatus <= 0) {
        unlockUser();
        goto exit;
    }
    uint16_t strLength = getStringLength(&lrq);
    char name[NAME_MAX];
    memcpy(name, lrq.body.lrq.name, strLength);
    //TODO: Send LoginResponse
    Message lre = initMessage(loginResponseCode);
    lre.body.lre.code = checkClientName(name, strLength);
    char *sName = "reference-server";
    uint16_t sNameLen = strlen(sName);
    memcpy(lre.body.lre.sName, sName, sNameLen);

    setMsgLength(&lre, strlen(sName));
    prepareMessage(&lre);
    sendMessage(self->sock, &lre);
    if (lre.body.lre.code != 0) {
        unlockUser();
        goto exit;
    }
    name[strLength] = '\0';
    strcpy(self->name, name);

    //TODO: Send UserAdded to all clients
    Message uad = initMessage(userAddedCode);
    memcpy(uad.body.uad.name, lrq.body.lrq.name, strLength);
    setMsgLength(&uad, strLength);
    uad.body.uad.timestamp = time(NULL);
    prepareMessage(&uad);
    broadcastMessage(NULL, &uad);

    if (debugEnabled() == 1) {
        debugPrint("List after a user was added:");
        printList();
    }


    //TODO: Send UserAdded of all previous registered clients to new client
    User *user = getFirstUser();
    while (user->next != NULL) {
        if (user != self) {
            strLength = strlen(user->name);
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
    int loop = 1;
    uint8_t urmCode;

    if (getChatStatus() == lock_sem) {
        s2c = initMessage(server2clientCode);
        char *text = "Chat paused by administrator.";
        strLength = strlen(text);
        s2c.body.s2c.timestamp = getTime();
        s2c.body.s2c.originalSender[0] = '\0';
        memcpy(s2c.body.s2c.text, text, strLength);
        setMsgLength(&s2c, strLength);
        prepareMessage(&s2c);
        broadcastMessage(NULL, &s2c);
    }
    while (loop == 1) {
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
            if (c2s.body.c2s.text[0] == '/') {
                handleAdmin(c2s, self);
            } else if (urmCode == kickedFromTheServerCode) {
                loop = 0;
            } else {
                strLength = getStringLength(&c2s);
                memcpy(s2c.body.s2c.text, c2s.body.c2s.text, strLength);
                strcpy(s2c.body.s2c.originalSender, self->name);
                s2c.body.s2c.timestamp = getTime();
                setMsgLength(&s2c, strLength);

                //TODO: Send Server2Client

                prepareMessage(&s2c);
                sendToQueue(&s2c, self);
            }

        }
    }
    lockUser();
    handleURM(urmCode, self);
    unlockUser();

    exit:
    debugPrint("Client thread stopping.");
    closeClient(self);
    return NULL;
}

int getStringLength(Message *buffer) {
    uint16_t length = 0;
    switch (buffer->header.type) {
        case loginRequestCode:
            length = buffer->header.length - sizeof(buffer->body.lrq.version) - sizeof(buffer->body.lrq.magic);
            debugPrint("String Length: %i", length);
            break;
        case client2ServerCode:
            length = buffer->header.length;
            break;
    }
    return length;
}

int checkClientName(char *name, int length) {
    int statusCode = otherServerErrorCode;
    for (int i = 0; i < length; i++) {
        if (name[i] < 33 || name[i] >= 126 || name[i] == 34 || name[i] == 37 || name[i] == 96) {
            statusCode = nameInvalidCode;
            return statusCode;
        } else {
            statusCode = successCode;
        }
    }
    User *user = getFirstUser();
    while (user != NULL) {
        if (strcmp(name, user->name) == 0) {
            statusCode = nameAlreadyTakenByAnotherUserCode;
            return statusCode;
        } else {
            statusCode = successCode;
        }
        user = user->next;
    }
    return statusCode;
}

void handleURM(uint8_t urmCode, User *self) {
    Message urm;
    uint16_t strLength;
    urm = initMessage(userRemovedCode);
    urm.body.urm.code = urmCode;
    urm.body.urm.timestamp = getTime();
    strLength = strlen(self->name);
    memcpy(urm.body.urm.name, self->name, strLength);
    setMsgLength(&urm, strLength);
    if (getFirstUser()->next != NULL) {
        prepareMessage(&urm);
        broadcastMessage(self, &urm);
    }
}

void handleAdmin(Message buffer, User *self) {
    Message s2c = initMessage(server2clientCode);
    char text[TEXT_MAX];

    uint16_t strLength = getStringLength(&buffer);
    memcpy(text, buffer.body.c2s.text, strLength);
    uint8_t commandCode;
    int isAdmin = strcmp(self->name, "Admin");

    //TODO: Identify the send command
    if (strncmp(text, "/kick", 5) == 0 && strLength > 6) {
        commandCode = kickClientCommandCode;
    } else if (strncmp(text, "/pause", 6) == 0) {
        commandCode = pauseChatCommandCode;
    } else if (strncmp(text, "/resume", 7) == 0) {
        commandCode = resumeChatCommandCode;
    } else {
        commandCode = invalidCommandCode;
    }
    debugPrint("Command: %i", commandCode);
    //TODO: Check the command
    if (commandCode == invalidCommandCode) {
        strcpy(text, "Invalid Command!");

    } else if (isAdmin != 0) {
        switch (commandCode) {
            case kickClientCommandCode:
                strcpy(text, "You must be administrator to use the /kick Command!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
                break;
            case pauseChatCommandCode:
                strcpy(text, "You must be administrator to use the /pause Command!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
                break;
            case resumeChatCommandCode:
                strcpy(text, "You must be administrator to use the /resume Command!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
                break;
            default:
                errorPrint("Invalid Input");
        }
    } else {
        if (commandCode == kickClientCommandCode) {
            char tbkName[NAME_MAX] = "";
            uint16_t length = getStringLength(&buffer);
            uint16_t nameLength = length - 6;
            memcpy(tbkName, buffer.body.c2s.text + 6, length);
            tbkName[length] = '\0';
            User *it = getFirstUser();
            while (it != NULL) {
                if (strcmp(tbkName, it->name) == 0) {
                    break;
                }
                it = it->next;
            }

            User *tbkUser = it;
            if (tbkUser == self) {
                strcpy(text, "Admin cannot be kicked from the server!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);

            } else if (tbkUser == NULL) {
                strcpy(text, "User to /kick does not exist on the server");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
            } else {
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
            if (getChatStatus() == lock_sem) {
                strcpy(text, "The chat is already paused!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
            } else {
                pauseChat();
                strcpy(text, "Chat paused by administrator.");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                broadcastMessage(NULL, &s2c);
            }
        }
        if (commandCode == resumeChatCommandCode) {
            if (getChatStatus() == unlock_sem) {
                strcpy(text, "The chat is not paused!");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                sendMessage(self->sock, &s2c);
            } else {
                resumeChat();
                strcpy(text, "The chat is no longer paused.");
                strLength = strlen(text);
                s2c.body.s2c.timestamp = getTime();
                s2c.body.s2c.originalSender[0] = '\0';
                memcpy(s2c.body.s2c.text, text, strLength);
                setMsgLength(&s2c, strLength);
                prepareMessage(&s2c);
                broadcastMessage(NULL, &s2c);
            }
        }
    }
}

void closeClient(User *user) {
    lockUser();
    deleteUser(user);
    unlockUser();
    if (debugEnabled() == 1) {
        debugPrint("List after a user was removed:");
        printList();
    }
    pthread_cancel(user->thread);
    close(user->sock);
    free(user);
}
