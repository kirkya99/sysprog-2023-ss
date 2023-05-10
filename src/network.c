#include <errno.h>
#include "network.h"
#include "util.h"

int networkReceive(int fd, Message *buffer)
{
    int connectionStatus;
    //TODO: Receive length
    connectionStatus = recv(fd, &buffer->header, sizeof(buffer->header), 0);
    if(connectionStatus == communicationError)
    {
        goto error;
    }
    else if(connectionStatus == clientClosedConnection)
    {
        return connectionStatus;
    }
    //TODO: Convert length byte order
    buffer->header.length = ntohs(buffer->header.length);
    //TODO: Validate header
    if(checkMsgHeader(buffer->header.type, buffer->header.length) < 0)
    {
        goto error;
    }
    //TODO: Receive body
    connectionStatus = recv(fd, &buffer->body, buffer->header.length, 0);
    if(connectionStatus == communicationError)
    {
        errorPrint("Error while receiving message!");
        goto error;
    }
    else if(connectionStatus == clientClosedConnection)
    {
        return connectionStatus;
    }
    if(buffer->header.type == loginRequestCode)
    {
        buffer->body.lrq.magic = ntohl(buffer->body.lrq.magic);
    }
    if(checkMsgBody(buffer) == -1)
    {
        goto error;
    }
    return connectionStatus;
    error:
    errno = ENOSYS;
    return -1;
}

int networkSend(int fd, const Message *buffer)
{
    //TODO: Send complete message
    int connectionStatus = send(fd, &buffer->header, sizeof(buffer->header), 0);
    if(connectionStatus <= communicationError)
    {
        goto error;
    }
    int length = ntohs(buffer->header.length);
    connectionStatus = send(fd, &buffer->body, length,0);
    if(connectionStatus <= communicationError)
    {
        goto error;
    }
    return connectionStatus;
    error:
    errno = ENOSYS;
    return -1;
}

int checkMsgHeader(uint8_t type, uint16_t length)
{
    int status = 0;
    if(type < 5)
    {
        errorPrint("Incorrect type!");
        status = -1;
    }
    switch (type) {
        case loginRequestCode:
            if(length <= 5 || length >= 37)
            {
                status = -2;
            }
            break;
        case client2ServerCode:
            if(length > 512)
            {
                status = -2;
            }
            break;
    }
    if(status == -2)
    {
        errorPrint("Incorrect length: %i!");
    }
    return status;
}

void closeConnectionToClient(int fd)
{
    close(fd);
}

int checkMsgBody(Message *buffer)
{
    int status = 0;
    if(buffer->header.type == loginRequestCode)
    {
        if(buffer->body.lrq.magic != 0x0badf00d)
        {
            errorPrint("Incorrect magic number!");
            status = -1;
        }
        if(buffer->body.lrq.version != 0)
        {
            errorPrint("Incorrect version!");
            status = -1;
        }
    }
    return status;
}

Message *initMessage(uint8_t msgType, Message *buffer)
{
    Message *newMessage;
    switch (msgType) {
        case loginResponseCode:
            newMessage->header.type = loginResponseCode;
            newMessage->body.lre.magic = htonl(0xc001c001);
            break;
        case server2clientCode:
            newMessage->header.type = server2clientCode;
            break;
        case userAddedCode:
            newMessage->header.type = userAddedCode;
            break;
        case userRemovedCode:
            newMessage->header.type = userRemovedCode;
            break;
    }
    return newMessage;
}


