#include <errno.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "server_defines.h"

mqd_t getQueueDescriptor(){
        struct mq_attr mqAttr;
        mqAttr.mq_flags = O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL;
        mqAttr.mq_maxmsg = 10;
        mqAttr.mq_msgsize = MSG_MAX;
        mqAttr.mq_curmsgs = 0;

    //create the server's queue
    errno = 0;
    mqd_t queueDesc = mq_open(serverQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
    if(queueDesc == -1){
        puts("Unable to open the server's queue. Perhaps it already exists - trying to delete it");
        puts(strerror(errno));
        mq_unlink(serverQueueName);
        queueDesc = mq_open(serverQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
        if(queueDesc == -1){
            puts("Failed to open the queue even after unlinking it.");
            exit(0);
        }
    } 
    return queueDesc;
}

int running = 1;

void intHandler(int signo){
    running = 0;
}

void handleConnect(clientList ** clients, char * buffer){
    //"connecter" ID then target ID
    int cnID, tID;
    sscanf(buffer, "%d %d", &cnID, &tID);
    clientList * snd = clientListFind(*clients, cnID);
    clientList * rcv = clientListFind(*clients, tID);
    if(snd == NULL)
        return;
    
    char sndBuff[MSG_MAX+1];
    sndBuff[MSG_MAX] = 0;

    if(rcv == NULL || cnID == tID || rcv->connected != 0){
        sprintf(sndBuff, "%d %s", -1, "NONE");
        mq_send(snd->queueID, sndBuff, MSG_MAX, CONNECT);
        return;
    }

    snd->connected = 1;
    rcv->connected = 1;
    sprintf(sndBuff, "%d %s", snd->clientID, snd->queueName);
    mq_send(rcv->queueID, sndBuff, MSG_MAX, CONNECT);
    sprintf(sndBuff, "%d %s", rcv->clientID, rcv->queueName);
    mq_send(snd->queueID, sndBuff, MSG_MAX, CONNECT);
}

void handleInit(clientList ** clients, char * buffer, int * nextClientID){
    //get client's queue name
    char clientQName[71];
    sscanf(buffer, "%70s", clientQName);
    mqd_t queueID = mq_open(clientQName, O_WRONLY | O_NONBLOCK);
    if(queueID < 0){
        puts("Init failed - unable to open queue.");
        return;
    }
    
    clientListInsert(clients, *nextClientID, queueID, clientQName);
    char sndBuff[MSG_MAX+1];
    sndBuff[MSG_MAX] = 0;
    sprintf(sndBuff, "%d\n", *nextClientID);
    mq_send(queueID, sndBuff, MSG_MAX, RESPONSE);
    (*nextClientID)++;
    printf("INIT handled (%s) -> %d\n", clientQName, *nextClientID-1);
}

void handleList(clientList ** clients, char * buffer){
    int clientID;
    sscanf(buffer, "%d", &clientID);
    clientList * clientData = clientListFind(*clients, clientID);
    if(!clientData)
        return;
    char responseBuffer[MSG_MAX+1];
    responseBuffer[MSG_MAX] = 0;
    responseBuffer[0] = 0;
    clientList * clCp = *clients;
    while(clCp != NULL){
        ssize_t oLen = strlen(responseBuffer);
        if(oLen > MSG_MAX - 120)
            break;
        if(clCp->connected)
            sprintf(responseBuffer + oLen, "%d - NOT AVAILABLE\n", clCp->clientID);
        else
            sprintf(responseBuffer + oLen, "%d - AVAILABLE\n", clCp->clientID);
        clCp = clCp->next;
    }
    sprintf(responseBuffer + strlen(responseBuffer), "End of client list\n");
    mq_send(clientData->queueID, responseBuffer, MSG_MAX, RESPONSE);
}

void handleDisconnect(clientList ** clients, char *  buffer){
    int ID1, ID2;
    sscanf(buffer, "%d %d", &ID1, &ID2);
    clientList * cl1 = clientListFind(*clients, ID1);
    clientList * cl2 = clientListFind(*clients, ID2);
    char b[MSG_MAX+1];
    b[0] = 0;
    if(cl1 != NULL){
        mq_send(cl1->queueID, b, MSG_MAX, DISCONNECT);
        cl1->connected = 0;
    }
    if(cl2 != NULL){
        mq_send(cl2->queueID, b, MSG_MAX, DISCONNECT);
        cl2->connected = 0;
    }
}

void handleStop(clientList ** clients, char * buffer){
    int clID;
    sscanf(buffer, "%d", &clID);
    clientList * cl = clientListFind(*clients, clID);
    if(!cl) 
        return;

    mq_close(cl->queueID);
    clientListRemove(clients, clID);
}

void serverLoop(mqd_t queueDesc){    
    messages messageType;

    char buffer[MSG_MAX+1];
    buffer[MSG_MAX] = 0;

    int nextClientID = 0;

    clientList * clients = NULL;

    while(running){
        if(mq_receive(queueDesc, buffer, MSG_MAX, &messageType) == -1){
            continue;
        }
        switch(messageType){  
            case STOP:
                puts("Handling STOP");
                handleStop(&clients, buffer);
                break;
            case DISCONNECT:
                puts("Handling disconnect");
                handleDisconnect(&clients, buffer);
                break;
            case LIST:
                puts("Handling LIST");
                handleList(&clients, buffer);
                break;
            case CONNECT:
                puts("Handling connect");
                handleConnect(&clients, buffer);
                break;
            case INIT:
                puts("HANDLING INIT");
                handleInit(&clients, buffer, &nextClientID);
                break;
            default:
                puts("Undefined request");
                break;
        }
    }

    clientList * tempC = clients;
    char nullMessage[MSG_MAX];
    memset(nullMessage, 0, MSG_MAX);
    while(tempC != NULL){
        mq_send(tempC->queueID, nullMessage, MSG_MAX, DISCONNECT);
        mq_send(tempC->queueID, nullMessage, MSG_MAX, SERVER_DOWN);
        mq_close(tempC->queueID);
        tempC = tempC->next;        
    }


    clientListFree(clients);
    mq_close(queueDesc);
    mq_unlink(serverQueueName);
}

int main(int argc, char ** argv){
    mqd_t queueDesc = getQueueDescriptor();
    signal(SIGINT, intHandler);
    puts("Server up and running");
    serverLoop(queueDesc);

    return 0;
}