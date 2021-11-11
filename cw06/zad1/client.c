#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include "defines.h"

int cligd;
int serverQ;
int myID;
int inChatMode = -1;

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

void chatWith(int anotherQ, int anotherID){
    inChatMode = anotherID;
    puts("Commands in chat mode:");
    puts("DISCONNECT - to disconnect");
    char * inpB = NULL;
    size_t inpS = 0;
    //set nonblocking mode
    struct{
        long rType;
        char data[MSGMAX+1];
    } request;
    request.data[MSGMAX] = 0;
    while(1){
        if(inputAvailable() > 0){
            getline(&inpB, &inpS, stdin);
            //send received input
            if(strncmp("DISCONNECT", inpB, strlen("DISCONNECT")) == 0){
                request.rType = DISCONNECT;
                sprintf(request.data, "%d %d", myID, anotherID);
                msgsnd(serverQ, &request, MSGMAX, 0);
            }
            else{
                request.rType = MESSAGE;
                strncpy(request.data, inpB, MSGMAX);
                msgsnd(anotherQ, &request, MSGMAX, 0);
            }
        }
        if(msgrcv(cligd, &request, MSGMAX, DISCONNECT, IPC_NOWAIT) > 0){
            //disconnect from the chat
            puts("Disconnecting");
            printf("\n>");
            fflush(stdout);
            inChatMode = -1;
            break;
        }
        if(msgrcv(cligd, &request, MSGMAX, MESSAGE, IPC_NOWAIT) > 0){
            printf("%d: ", anotherID);
            puts(request.data);
        }
    }

    free(inpB);
}

void sigintHandler(int signo){
  
    struct{
        long rType;
        char data[MSGMAX];
    } request;

    if(inChatMode != -1){
        request.rType = DISCONNECT;
        sprintf(request.data, "%d %d", myID, inChatMode);
        msgsnd(serverQ, &request, MSGMAX, 0);
    }

    puts("Quitting.");
    request.rType = STOP;
    sprintf(request.data, "%d", myID);
    msgsnd(serverQ, &request, MSGMAX, 0);    
    msgctl(cligd, IPC_RMID, 0);

    exit(0);
}

void clientLoop(int serverQ, int myQ, int myID){
    struct{
        long rType;
        char data[MSGMAX];
    } request;
    char *inpB = NULL;
    char command[100];
    int param;
    size_t nRead = 0;
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    printf(">");
    fflush(stdout);
    while(1){
        if(inputAvailable() > 0){
            getline(&inpB, &nRead, stdin);
            if(sscanf(inpB, "%99s %d", command, &param) < 1)
                continue;
            if(strcmp(command, "LIST") == 0){
                request.rType = LIST;
                sprintf(request.data, "%d", myID);
                msgsnd(serverQ, &request, MSGMAX, 0);
                msgrcv(myQ, &request, MSGMAX, RESPONSE, 0);
                puts(request.data);
            }
            else if(strncmp(command, "CONNECT", strlen("CONNECT")) == 0){
                request.rType = CONNECT;
                int connectTo = param;
                sprintf(request.data, "%d %d", myID, connectTo);
                msgsnd(serverQ, &request, MSGMAX, 0);
            }
            else if(strcmp(command, "STOP") == 0){
                sigintHandler(SIGINT);
            }
            else{
                puts("Available commands: LIST | STOP | CONNECT [ID]");
            }
            printf("\n>");
            fflush(stdout);
        }
        if(msgrcv(myQ, &request, MSGMAX, CONNECT, IPC_NOWAIT) > 0){
            int qId, hId;
            sscanf(request.data, "%d %d", &qId, &hId);
            if(qId == -1){
                puts("Unable to connect to selected client.");
                printf("\n>");
                fflush(stdout);
            }
            else{
                chatWith(qId, hId);
            }
        }
        if(msgrcv(myQ, &request, MSGMAX, SERVER_DOWN, IPC_NOWAIT) > 0){
            puts("Server is shutting down...");
            sigintHandler(SIGINT);
        }

    }
    free(inpB);
}

int main(int argc, char ** argv){
    struct passwd * pw = getpwuid(getuid());

    srand(time(NULL));
    puts("Starting client");
    int msgd = msgget(SERVER_IPC_KEY, 0);
    if(msgd == -1){
        puts("Unable to open server's queue");
        return 0;
    }
    //open a queue and send an init
    key_t qKey = ftok(pw->pw_dir, rand() % 256);
    cligd = msgget(qKey, IPC_CREAT | IPC_EXCL | 0666);
    if(cligd == -1){
        puts("Unable to open queue. Try again.");
        return 0;
    }
    printf("Client started with msgID %d.\n", cligd);
    serverQ = msgd;
    signal(SIGINT, sigintHandler);

    struct{
        long type;
        char req[MSGMAX];
    } init_request;
    init_request.type = INIT;
    sprintf(init_request.req, "%d", cligd);
    msgsnd(msgd, &init_request, 128, IPC_NOWAIT);
    //now receive from server
    puts("Waiting for server's response");
    msgrcv(cligd, &init_request, MSGMAX, RESPONSE, 0);
    sscanf(init_request.req, "%d", &myID);
    printf("Server responded. My ID is: %d\n", myID);
    clientLoop(msgd, cligd, myID);


    msgctl(cligd, IPC_RMID, 0);

    return 0;
}