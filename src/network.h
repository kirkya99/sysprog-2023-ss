#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */

enum MESSAGE_TYPES {
    loginRequestCode = 0,
    loginResponseCode = 1,
    client2ServerCode = 2,
    server2clientCode = 3,
    userAddedCode = 4,
    userRemovedCode = 5
};
enum LOGIN_RESPONSE_CODES {
    successCode = 0,
    nameAlreadyTakenByAnotherUserCode = 1,
    nameInvalidCode = 2,
    protocolVersionMismatchCode = 3,
    otherServerErrorCode = 255
};
enum USER_REMOVED_CODES {
    connectionClosedByClientCode = 0,
    kickedFromTheServerCode = 1,
    communicationErrorCode = 2
};

enum NETWORK_STATUS {
    clientClosedConnection = 0,
    communicationError = -1
};
enum MESSAGE_LIMITS {
    TEXT_MAX = 512,
    NAME_MIN = 1,
    NAME_MAX = 31,
    SENDER_MAX = 32,
    HEADER_MAX = 3,
    BODY_MAX = 552,
    MSG_MAX = HEADER_MAX + BODY_MAX + 1
};


/*
typedef struct __attribute__((packed))
{
    uint16_t len;		//real length of the text (big endian, len <= MSG_MAX)
    char text[MSG_MAX];	//text message
} Message;
*/
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint16_t length;
} Header;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t version;
    char name[NAME_MAX];
} LoginRequest;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t code;
    char sName[NAME_MAX];
} LoginResponse;

typedef struct __attribute__((packed)) {
    char text[TEXT_MAX];
} Client2Server;

typedef struct __attribute__((packed)) {
    uint64_t timestamp;
    char originalSender[SENDER_MAX];
    char text[TEXT_MAX];
} Server2Client;

typedef struct __attribute__((packed)) {
    uint64_t timestamp;
    char name[NAME_MAX];
} UserAdded;

typedef struct __attribute__((packed)) {
    uint64_t timestamp;
    uint8_t code;
    char name[NAME_MAX];
} UserKicked;

typedef union __attribute__((packed)) {
    LoginRequest lrq;
    LoginResponse lre;
    Client2Server c2s;
    Server2Client s2c;
    UserAdded uad;
    UserKicked urm;
} Body;

typedef struct __attribute__((packed)) {
    Header header;
    Body body;
} Message;

int networkReceive(int fd, Message *buffer);

int networkSend(int fd, const Message *buffer);

int checkMsgHeader(uint8_t type, uint16_t length);

int checkMsgBody(Message *buffer);

Message initMessage(uint8_t msgType);

void setMsgLength(Message *buffer, uint16_t strLength);

void prepareMessage(Message *buffer);

uint64_t getTime();

#endif
