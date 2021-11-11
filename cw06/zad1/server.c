#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <signal.h>
#include "server-defines.h"

int msgOpen(char ** argv){
    errno = 0;
    key_t ipcKey = SERVER_IPC_KEY;
    msgctl(ipcKey, IPC_RMID, NULL);
    int msgDesc = msgget(ipcKey, IPC_CREAT | IPC_EXCL | 0666);
    if(msgDesc == -1){
        int stillFailed = 0;
        if(errno == EEXIST){
            msgDesc = msgget(ipcKey, 0);
            if(msgctl(msgDesc, IPC_RMID, 0) == -1)
                stillFailed = 1;
            else
                msgDesc = msgget(ipcKey, IPC_CREAT | IPC_EXCL | 0666);
        }
        if(stillFailed)
            return -1;
    }
    return msgDesc;
}

int running = 1;

void exitHandler(int signo){
    running = 0;
}

void handleList(int msgKey, clientList * clients, char * inputBuffer){
    int clientID;
    sscanf(inputBuffer, "%d", &clientID);
    clientList * clientData = clientListFind(clients, clientID);
    if(!clientData)
        return;
    struct{
        long mtype;
        char output[MSGMAX];
    } response;
    response.mtype = RESPONSE;
    response.output[MSGMAX-1] = 0;
    response.output[0] = 0;
    while(clients != NULL){
        size_t oLen = strlen(response.output);
        if(oLen > MSGMAX - 120)
            break;
        if(clients->connected)
            sprintf(response.output + oLen, "%d - NOT AVAILABLE\n", clients->clientID);
        else
            sprintf(response.output + oLen, "%d - AVAILABLE\n", clients->clientID);
        clients = clients->next;
    }
    sprintf(response.output + strlen(response.output), "End of client list.\n");  
    msgsnd(clientData->queueID, &response, MSGMAX, IPC_NOWAIT);  
}

void handleStop(clientList ** clients, char * inputBuffer){
    int clId;
    sscanf(inputBuffer, "%d", &clId);
    clientListRemove(clients, clId);
}

void handleConnect(clientList ** clients, char * inputBuffer){
    struct{
        long mtype;
        char output[MSGMAX];
    } response;
    int clId; int cnId;
    sscanf(inputBuffer, "%d %d", &clId, &cnId);
    clientList * snd = clientListFind(*clients, clId);
    clientList * rcv = clientListFind(*clients, cnId);
    if(snd == NULL)
        return;

    if(rcv == NULL || clId == cnId || rcv->connected != 0){
        response.mtype = CONNECT;
        sprintf(response.output, "%d %d", -1, -1);
        msgsnd(snd->queueID, &response, MSGMAX, IPC_NOWAIT);
        return;
    }

    snd->connected = 1;
    rcv->connected = 1;
    response.mtype = CONNECT;
    sprintf(response.output, "%d %d", snd->queueID, clId);
    msgsnd(rcv->queueID, &response, MSGMAX, IPC_NOWAIT);
    sprintf(response.output, "%d %d", rcv->queueID, cnId);
    msgsnd(snd->queueID, &response, MSGMAX, IPC_NOWAIT);
}

void handleInit(int * nextClientID, clientList ** clients, char * inputBuffer){
    //open clients queue
    //add the client to list
    //open client's queue
    int clientQ; 
    sscanf(inputBuffer, "%d", &clientQ);
    clientListInsert(clients, *nextClientID, clientQ);
    struct{
        long mtype;
        char buffer[60];
    }clientData;
    clientData.mtype = RESPONSE;
    sprintf(clientData.buffer, "%d\n", *nextClientID);
    //send client id to him
    printf("Responding to init. Received queue id: %d\n", clientQ);
    msgsnd(clientQ, &clientData, strlen(clientData.buffer) + 1, IPC_NOWAIT);
    (*nextClientID)++;
}

void handleDisconnect(clientList ** clients, char * inputBuffer){
    struct{
        long mtype;
        char output[MSGMAX];
    } response;
    int id0; int id1;
    sscanf(inputBuffer, "%d %d", &id0, &id1);
    clientList * cl0 = clientListFind(*clients, id0);
    clientList * cl1 = clientListFind(*clients, id1);    
    response.mtype = DISCONNECT;
    response.output[0] = 0;
    if(cl0 != NULL){
        msgsnd(cl0->queueID, &response, MSGMAX, IPC_NOWAIT);
        cl0->connected = 0;
    }
    if(cl1 != NULL){
        msgsnd(cl1->queueID, &response, MSGMAX, IPC_NOWAIT);
        cl1->connected = 0;
    }

}

void serverLoop(int msgKey){
    puts("Server started.");
    struct{
        long mtype;
        char buffer[MSGMAX+1];
    } msgData;
    int nextClientID = 0;
    clientList * clients = NULL;
    while(running){
        puts("Listening for messages.");
        size_t nRead = msgrcv(msgKey, &msgData, sizeof(msgData) - sizeof(long), -100, MSG_NOERROR);
        puts("Received message");
        msgData.buffer[nRead] = 0;
        switch(msgData.mtype){
            case STOP:
                puts("Handling STOP");
                handleStop(&clients, msgData.buffer);
                break;
            case DISCONNECT:
                puts("Handling disconnect");
                handleDisconnect(&clients, msgData.buffer);
                break;
            case LIST:
                puts("Handling LIST");
                handleList(msgKey, clients, msgData.buffer);
                break;
            case CONNECT:
                puts("Handling connect");
                handleConnect(&clients, msgData.buffer);
                break;
            case INIT:
                puts("HANDLING INIT");
                handleInit(&nextClientID, &clients, msgData.buffer);
                break;
        }
    }
    clientList * tempL = clients;
    while(tempL != NULL){
        msgData.mtype = SERVER_DOWN;
        msgsnd(tempL->queueID, &msgData, MSGMAX, IPC_NOWAIT);
        msgData.mtype = DISCONNECT;
        msgsnd(tempL->queueID, &msgData, MSGMAX, IPC_NOWAIT);
        tempL = tempL->next;
    }
    clientListFree(clients);
    msgctl(msgKey, IPC_RMID, NULL);
}

int main(int argc, char ** argv){
    int msgKey = msgOpen(argv);
    if(msgKey == -1){
        printf("Failed to open message queue: %s\n", strerror(errno));
        return 0;
    }

    signal(SIGINT, exitHandler);

    serverLoop(msgKey);

    return 0;
}