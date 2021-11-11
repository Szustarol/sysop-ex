#include "defines.h"
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>

mqd_t myQueueDesc;
int myID;
mqd_t serverQueueDesc;
mqd_t anotherQ;
char conBuff[100];
char myQueueName[200];
char messageBuffer[MSG_MAX+1];
int inChatMode = -1;

mqd_t createMyQueue(){
    pid_t myPid = getpid();
    sprintf(myQueueName, "/clientQueue_%d", myPid);

    struct mq_attr mqAttr;
    mqAttr.mq_flags = O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL;
    mqAttr.mq_maxmsg = 10;
    mqAttr.mq_msgsize = MSG_MAX;
    mqAttr.mq_curmsgs = 0;

    mqd_t queueDesc = mq_open(myQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
    if(queueDesc == -1){
        puts(strerror(errno));
        puts("Unable to open client's queue. Perhaps it exists - trying to delete it");
        mq_unlink(myQueueName);
        queueDesc = mq_open(myQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
        if(queueDesc == -1){
            puts("Failed to open the queue after removing it.");
            return -1;
        }
    }
    return queueDesc;
}

int inputAvailable(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return (FD_ISSET(0,  &fds));
}

void chatWith(char * anotherQName, int anotherID){
    anotherQ = mq_open(anotherQName, O_WRONLY | O_NONBLOCK);
    if(anotherQ == -1)
        return;
    inChatMode = anotherID;
    puts("Commands in chat mode:");
    puts("DISCONNECT - to disconnect");
    char * inpB = NULL;
    size_t inpS = 0;
    ssize_t nRead;
    while(1){
        if(inputAvailable() > 0){
            getline(&inpB, &inpS, stdin);
            if(strncmp("DISCONNECT", inpB, strlen("DISCONNECT")) == 0){
                sprintf(messageBuffer, "%d %d", myID, anotherID);
                mq_send(serverQueueDesc, messageBuffer, MSG_MAX, DISCONNECT);
            }
            else{
                strncpy(messageBuffer, inpB, MSG_MAX);
                mq_send(anotherQ, messageBuffer, MSG_MAX, MESSAGE);
            }
        }
        unsigned prio;
        errno = 0;
        nRead = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, &prio);
        if(errno == 0 && nRead > 0){
            if(prio == DISCONNECT){
                puts("Disconnecting");
                printf("\n>");
                fflush(stdout);
                mq_close(anotherQ);
                inChatMode = -1;
                break;
            }
            else if(prio == MESSAGE){
                printf("%d: ", anotherID);
                puts(messageBuffer);
            }
        }
    }
    free(inpB);
}

void intHandler(int signo){
    if(inChatMode != -1){
        sprintf(messageBuffer, "%d %d", myID, inChatMode);
        mq_send(serverQueueDesc, messageBuffer, MSG_MAX, DISCONNECT);
        mq_close(anotherQ);
    }

    puts("Quitting");
    sprintf(messageBuffer, "%d", myID);
    mq_send(serverQueueDesc, messageBuffer, MSG_MAX, STOP);
    mq_close(serverQueueDesc);
    mq_close(myQueueDesc);
    mq_unlink(myQueueName);
    exit(0);
}

void clientLoop(){
    char * inpB = NULL;
    size_t inpS = 0;
    char command[100];
    int param;
    size_t nRead = 0;
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    printf(">");
    fflush(stdout);
    while(1){
        if(inputAvailable() > 0){
            getline(&inpB, &inpS, stdin);
            if(sscanf(inpB, "%99s %d", command, &param) < 1)
                continue;
            if(strcmp(command, "LIST") == 0){
                sprintf(messageBuffer, "%d", myID);
                mq_send(serverQueueDesc, messageBuffer, MSG_MAX, LIST);
                do{
                    errno = 0;
                    nRead = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, NULL);
                }while(errno == EAGAIN || nRead == 0);
                puts(messageBuffer);
            }
            else if(strncmp(command, "CONNECT", strlen("CONNECT")) == 0){
                sprintf(messageBuffer, "%d %d", myID, param);
                mq_send(serverQueueDesc, messageBuffer, MSG_MAX, CONNECT);
            }
            else if(strcmp(command, "STOP") == 0){
                intHandler(SIGINT);
            }
            else{
                puts("Available commands: LIST | STOP | CONNECT [ID]");
            }
            printf("\n>");
            fflush(stdout);
        }
        unsigned prio;
        errno = 0;
        nRead = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, &prio);
        if(errno == 0 && nRead > 0){
            if(prio == CONNECT){
                int hId;
                sscanf(messageBuffer, "%d %s", &hId, conBuff);
                if(hId == -1){
                    puts("Unable to connect to selected client.");
                    printf("\n>");
                    fflush(stdout);
                }
                else{
                    chatWith(conBuff, hId);
                }
            }
            else if(prio == SERVER_DOWN){
                puts("Server is shutting down...");
                intHandler(SIGINT);
            }
        }
    }
    free(inpB);
}

int main(int argc, char ** argv){
    messageBuffer[MSG_MAX] = 0;
    messageBuffer[0] = 0;

    serverQueueDesc = mq_open(serverQueueName, O_WRONLY);
    if(serverQueueDesc < 0){
        puts("Unable to upen server's queue.");
        return 0;
    }

    if((myQueueDesc = createMyQueue()) == -1){
        return 0;
    }
    printf("Client started with queue name %s\n", myQueueName);

    signal(SIGINT, intHandler);
    sprintf(messageBuffer, "%s", myQueueName);
    mq_send(serverQueueDesc, messageBuffer, MSG_MAX, INIT);
    puts("Waiting for server's response");
    ssize_t nRecv = 0;
    do{
        errno = 0;
        nRecv = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, NULL);
    }while(errno == EAGAIN || nRecv == 0);
    sscanf(messageBuffer, "%d", &myID);
    printf("Server responded. My ID is: %d\n", myID);

    clientLoop();

    return 0;
}