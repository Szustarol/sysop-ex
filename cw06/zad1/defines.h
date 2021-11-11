#ifndef DEFINES_H
#define DEFINES_H
#define MSGMAX 4096
#include <stdlib.h>
#include <string.h>

#define SERVER_IPC_KEY 66433

const int proj_ids[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19
};

const int nProjIDs = 20;

typedef enum{
    STOP = 1,
    DISCONNECT,
    LIST,
    CONNECT,
    INIT,
    RESPONSE,
    MESSAGE,
    SERVER_DOWN
}messages; //keeps track of priority

#endif