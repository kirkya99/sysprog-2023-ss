#include <errno.h>
#include "network.h"

int networkReceive(int fd, Message *buffer)
{
    int connectionStatus;
    //TODO: Receive length
    connectionStatus = recv(fd, buffer, sizeof(buffer->len), 0);
    if(connectionStatus <= communicationError)
    {
        goto error;
    }
    else if(connectionStatus == clientClosedConnection)
    {
        return connectionStatus;
    }
    //TODO: Convert length byte order
    buffer->len = ntohs(buffer->len);
    //TODO: Validate length
    if(buffer->len > MSG_MAX)
    {
        goto error;
    }
    //TODO: Receive text
    recv(fd, buffer->text, buffer->len, 0);
    if(connectionStatus <= communicationError)
    {
        goto error;
    }
    else if(connectionStatus == clientClosedConnection)
    {
        return connectionStatus;
    }
    return connectionStatus;
    error:
    errno = ENOSYS;
    return -1;
}

int networkSend(int fd, const Message *buffer)
{
    //TODO: Send complete message

    int connectionStatus = send(fd, buffer, temp, 0);
    if(connectionStatus <= communicationError)
    {
        goto error;
    }
    return connectionStatus;
    error:
    errno = ENOSYS;
    return -1;
}

