#include <errno.h>
#include "connectionhandler.h"
#include "util.h"
#include "user.h"
#include "clientthread.h"

static int createPassiveSocket(in_port_t port) {
    int fd = -1;
    const int backlog = 4;
    //TODO: socket()
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        goto error;
    }
    const int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    //TODO: bind() to port
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr_in *) &address, sizeof(address)) == -1) {
        goto error;
    }
    //TODO: listen()
    if (listen(fd, backlog) == -1) {
        goto error;
    }

    return fd;
    error:
    errno = ENOSYS;
    return -1;
}

int connectionHandler(in_port_t port) {
    const int fd = createPassiveSocket(port);
    if (fd == -1) {
        errnoPrint("Unable to create server socket");
        return -1;
    }
    if (initMutex() != 0) {
        errnoPrint("Error while initializing mutex");
        return -1;
    }
    for (;;) {
        //TODO: accept() incoming connection
        const int active_socket = accept(fd, NULL, NULL);
        if (active_socket == -1) {
            errnoPrint("Error accepting a connection");
            continue;
        }
        //TODO: add connection to user list and start client thread
        lockUser();
        struct User *user = createUser();
        user->sock = active_socket;
        // createThread:
        if (pthread_create(&user->thread, NULL, clientthread, user) != 0) {
            errorPrint("Error while creating thread");
            continue;
        }
        unlockUser();
        continue;
        return 0;    //never reached
    }
}