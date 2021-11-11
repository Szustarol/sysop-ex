#ifndef DEFINES_H
#define DEFINES_H
#include <stdlib.h>
#include <string.h>
#define MSG_MAX 4096
const char serverQueueName [] = "/posixServerQueue";

typedef enum{
    SERVER_DOWN, 
    MESSAGE,
    RESPONSE,
    INIT,
    CONNECT,
    LIST,
    DISCONNECT, 
    STOP
}messages;

#endif