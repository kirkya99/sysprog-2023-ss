#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>


/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */
enum
{
    MSG_MAX = 1024
};
/*
enum TEXT_LIMIT
{
    TEXT_MAX = 512
};

enum NAME_LIMIT
{
    NAME_MIN = 1,
    NAME_MAX = 31
};
enum MESSAGE_TYPES
{
    loginRequestCode = 0,
    loginResponseCode = 1,
    client2ServerCode = 2,
    server2clientCode = 3,
    userAddedCode = 4,
    userRemovedCode = 5
};
enum LOGIN_RESPONSE_CODES
{
    successCode = 0,
    nameAlreadyTakenByAnotherUserCode = 1,
    nameInvalidCode = 2,
    protocolVersionMismatchCode = 3,
    otherServerErrorCode = 255
};
enum USER_REMOVED_CODES
{
    connectionClosedByClientCode = 0,
    kickedFromTheServerCode = 1,
    communicationErrorCode = 2
};
 */
enum NETWORK_STATUS
{
    clientClosedConnection = 0,
    communicationError = -1
};



typedef struct __attribute__((packed))
{
    uint16_t len;		//real length of the text (big endian, len <= MSG_MAX)
    char text[MSG_MAX];	//text message
} Message;

int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);

#endif
