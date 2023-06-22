#include <pthread.h>
#include "user.h"
#include "clientthread.h"
#include "util.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

//TODO: Implement the functions declared in user.h

/*
 * User functions
 */
struct User *createUser() {
    User *newUser = (User *) malloc(sizeof(User));
    newUser->sock = 0;
    newUser->thread = 0;
    if (newUser == NULL) {
        return NULL;
    }

    if (userFront == NULL) {
        newUser->prev = NULL;
        newUser->next = NULL;
        userFront = newUser;
        userBack = newUser;
    } else if (userFront != NULL) {
        userBack->next = newUser;
        newUser->next = NULL;
        newUser->prev = userBack;
        userBack = newUser;
    }
    return newUser;
}

int iterateOverList(User *self, void *buffer, void (*func)(int, void *)) {
    int retVal = 0;
    for (User *it = userFront; it != NULL; it = it->next) {
        if (it != self) {
            func(it->sock, buffer);
        }
    }
    return retVal;
}

void deleteUser(User *toBeDeleted) {
    if (toBeDeleted == userFront) {
        userFront = toBeDeleted->next;
    } else {
        toBeDeleted->prev->next = toBeDeleted->next;
    }

    if (toBeDeleted == userBack) {
        userBack = toBeDeleted->prev;
    } else {
        toBeDeleted->next->prev = toBeDeleted->prev;
    }

}

struct User *getFirstUser() {
    return userFront;
}

/*
 * Mutex Locking
 */
void lockUser() {
    pthread_mutex_lock(&userLock);
}

void unlockUser() {
    pthread_mutex_unlock(&userLock);
}

int initMutex() {
    int ret;
    pthread_mutexattr_t mattr;
    ret = pthread_mutex_init(&userLock, &mattr);
    return ret;
}

void printList() {
    User *it = getFirstUser();
    if(it==NULL)
    {
        debugPrint("-");
    }
    while (it != NULL) {
        debugPrint(it->name);
        it = it->next;
    }
}

