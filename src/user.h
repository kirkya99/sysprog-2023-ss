#ifndef USER_H
#define USER_H

#include <pthread.h>
#include <stdlib.h>

typedef struct User {
    struct User *prev;
    struct User *next;
    pthread_t thread;    //thread ID of the client thread
    int sock;        //socket for client
    char name[32];
} User;

//TODO: Add prototypes for functions that fulfill the following tasks:
// * Add a new user to the list and start client thread
// * Iterate over the complete list (to send messages to all users)
// * Remove a user from the list
//CAUTION: You will need proper locking!

/*
 * User functions
 */
struct User *createUser();

int iterateOverList(User *self, void *buffer, void (*func)(int, void *));

void deleteUser(User *toBeDeleted);

struct User *getFirstUser();

void printList();

/*
 * Mutex Locking
 */
void lockUser();

void unlockUser();

int initMutex();

#endif
