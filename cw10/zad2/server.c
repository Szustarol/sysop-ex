#include "serverClients.h"
#include "defines.h"
#include <string.h>

//clientslist is now only used by the handling thread
serverClientsList * clientsList;

messageQueue  * messages = NULL;
pthread_mutex_t messagesMutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in webSockStruct;
struct sockaddr_un localSockStruct;

#define NNOTRESPONDING 40
#define NNOTRESPONDING_CHECK_DELAY 80

int unixSocket;
int webSocket;

void * handleIncomingLinux(void * dummy){
    char incomingBuffer[BUF_SIZE];
    struct sockaddr incoming;
    socklen_t socksize = sizeof(incoming);
    while(1){
        ssize_t nrec = recvfrom(unixSocket, incomingBuffer, BUF_SIZE, 0, &incoming, &socksize);
        char * msg = malloc(nrec+1);
        msg[nrec] = 0;
        memcpy(msg, incomingBuffer, nrec);
        pthread_mutex_lock(&messagesMutex);
        mesList_add(&messages, msg, incoming, LOCAL);
        pthread_mutex_unlock(&messagesMutex);
    }

}

void * handleIncomingWeb(void * dummy){
    char incomingBuffer[BUF_SIZE];
    struct sockaddr incoming;
    socklen_t socksize = sizeof(incoming);
    while(1){
        ssize_t nrec = recvfrom(webSocket, incomingBuffer, BUF_SIZE, 0, &incoming, &socksize);
        char * msg = malloc(nrec+1);
        msg[nrec] = 0;
        memcpy(msg, incomingBuffer, nrec);
        pthread_mutex_lock(&messagesMutex);
        mesList_add(&messages, msg, incoming, WEB);
        pthread_mutex_unlock(&messagesMutex);
    }
}

void sendString(const char * string, struct sockaddr cliaddr, ClientConnectionType ct){
    if(ct == WEB){
        sendto(webSocket, string, strlen(string), 0, &cliaddr, sizeof(struct sockaddr_in));
    }
    else if(ct == LOCAL){
        sendto(unixSocket, string, strlen(string), 0, &cliaddr, sizeof(struct sockaddr_un));
    }
}

void registerClient(messageQueue * mq){
    //extract name
    char * name = mq->message+1;
    if(cliList_find(clientsList, name) != NULL){ //client exists
        sendString("TAKEN", mq->sock, mq->type);
        return;
    }
    sendString("OK", mq->sock, mq->type);
    cliList_insert(&clientsList, name, mq->type, mq->sock, -1);
    printf("New client registered with name %s\n", name);
}

void handlePONG(messageQueue * mq){
    //find the client
    serverClientsList * client = cliList_findBySock(clientsList, &mq->sock);
    if(client == NULL)
        return;
    client->nNotResponding = 0;
}

void handleDC(messageQueue * mq){
    //find the client
    serverClientsList * client = cliList_findBySock(clientsList, &mq->sock);
    if(client == NULL)
        return;
    //if the client is connected send DC signal to the other player
    if(client->connectedToName != NULL){
        serverClientsList  * conClient = cliList_find(clientsList, client->connectedToName);
        if(conClient != NULL){
            sendString("DC", conClient->addr, conClient->connectionType);
            conClient->connectedToName = NULL;
        }
    }
    cliList_delete(&clientsList, client->clientName);
}

void handleGameover(messageQueue * mq){
    //find the client
    serverClientsList * client = cliList_findBySock(clientsList, &mq->sock);
    if(client == NULL)
        return;
    client->connectedToName = NULL;
}

void handleBoardState(messageQueue * mq){
    //find the client
    serverClientsList * client = cliList_findBySock(clientsList, &mq->sock);
    if(client == NULL)
        return;
    //if the client is connected send DC signal to the other player
    serverClientsList  * conClient = cliList_find(clientsList, client->connectedToName);
    if(conClient != NULL){
        sendString(mq->message, conClient->addr, conClient->connectionType);
        printf("Received board state from client %s, passing to %s\n", client->clientName, client->connectedToName);
    }
}

void handlePair(serverClientsList * fcl, serverClientsList * scl){
    fcl->connectedToName = scl->clientName;
    scl->connectedToName = fcl->clientName;
    char sndbuf[BUF_SIZE];
    sprintf(sndbuf, "X: Connected with %s as X\n", fcl->clientName);
    sendString(sndbuf, scl->addr, scl->connectionType);
    sprintf(sndbuf, "O: Connected with %s as O\n", scl->clientName);
    sendString(sndbuf, fcl->addr, fcl->connectionType);
    printf("Paired %s with %s\n", fcl->clientName, scl->clientName);
}

void * handleGames(void * dummy){
    int lastTime = getTimeMs();

    while(1){
        //parse each incoming message
        while(1){
            pthread_mutex_lock(&messagesMutex);
            if(mesList_isEmpty(&messages)) {
                pthread_mutex_unlock(&messagesMutex);
                break;
            }

            messageQueue * mes = mesList_get(&messages);

            pthread_mutex_unlock(&messagesMutex);

            //if incoming is :<something> this means a new client wants to register
            if(mes->message[0] == ':'){
                registerClient(mes);
            }
            //if incoming message is "DC" this means the client is disconnected
            else if(strcmp(mes->message, "DC") == 0){
                handleDC(mes);
            }
            //if the incoming message is "PONG" this means the client responded to ping
            else if(strcmp(mes->message, "PONG") == 0){
                handlePONG(mes);
            }
            else if(strcmp(mes->message, "GAMEOVER") == 0){
                handleGameover(mes);
            }
            //the client sent a board state
            else{
                handleBoardState(mes);
            }

            mesList_destroy(mes);
        }

        //try to pair two clients
        serverClientsList * fcl = NULL;
        serverClientsList * scl = NULL;
        serverClientsList * client = clientsList;
        while(client != NULL){
            if(client->connectedToName == NULL){
                if(fcl == NULL){
                    fcl = client;
                }
                else{
                    scl = client;
                    break;
                }
            }
            client = client->next;
        }
        if(fcl != NULL && scl != NULL){
            handlePair(fcl, scl);
        }

        if(getTimeMs() -lastTime > NNOTRESPONDING_CHECK_DELAY) {
            lastTime = getTimeMs();
            client = clientsList;
            while (client != NULL) {
                if (client->connectedToName == NULL) {
                    sendString("PING", client->addr, client->connectionType);
                    if (client->nNotResponding > NNOTRESPONDING) {
                        printf("Client %s is not responding. Closing connection.\n", client->clientName);
                        cliList_delete(&clientsList, client->clientName);
                        break;
                    }
                }
                client = client->next;
            }
        }

    }
}

void unbind(void){
    close(webSocket);
    close(unixSocket);
}

int main(int argc, char ** argv){
    signal(SIGPIPE, SIG_IGN);

    clientsList = cliList_create();

    if(argc < 3){
        puts("Invalid number of arguments");
        return 0;
    }

    short portNum;
    char * unixPath = argv[2];

    if((portNum = (short)atoi(argv[1])) == 0){
        puts("Invalid port number.");
        return 0;
    }

    puts("Creating sockets...");

    if((webSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        printf("Unable to create socket at port %d\n", portNum);
        return 0;
    }

    if((unixSocket = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1){
        printf("Unable to create unix socket\n");
        return 0;
    }

    puts("Binding sockets...");

    unlink(unixPath);
    strcpy(localSockStruct.sun_path, unixPath);
    localSockStruct.sun_family = AF_LOCAL;
    if(bind(unixSocket, (struct sockaddr * )&localSockStruct, sizeof(localSockStruct)) == -1){
        puts("Unable to bind local socket");
        return 0;
    }

    webSockStruct.sin_family = AF_INET;
    webSockStruct.sin_port = htons(portNum);
    webSockStruct.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(webSocket, (struct sockaddr * )&webSockStruct, sizeof(webSockStruct) )== -1){
        puts("Unable to bind web socket");
        return 0;
    }


    atexit(unbind);

    pthread_t incomingLinux, incomingWeb, handlingThread;

    pthread_create(&incomingWeb, NULL, handleIncomingWeb, NULL);

    pthread_create(&handlingThread, NULL, handleGames, NULL);

    pthread_create(&incomingLinux, NULL, handleIncomingLinux, NULL);

    pthread_join(incomingLinux, NULL);
    pthread_join(handlingThread, NULL);
    pthread_join(incomingWeb, NULL);

    return 0;
}