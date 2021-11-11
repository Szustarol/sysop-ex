//
// Created by karolszustakowski on 24.05.2021.
//

#ifndef CW10_SERVERCLIENTS_H
#define CW10_SERVERCLIENTS_H
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>

typedef enum{
    LOCAL,
    WEB,
    NONE
}ClientConnectionType;

struct __serverClientsList;

typedef struct __serverClientsList{
    char * clientName;
    char * connectedToName;
    ClientConnectionType connectionType;
    int confd;
    int nNotResponding;
    struct sockaddr addr;
    struct __serverClientsList * next;
}serverClientsList;

serverClientsList * cliList_create(){
    return NULL;
}

void cliList_insert(serverClientsList ** cliList, char * clientName, ClientConnectionType connectionType, struct sockaddr addr, int confd){
    serverClientsList *newEntry = malloc(sizeof (serverClientsList));
    char * cliName = malloc(sizeof(char)*(strlen(clientName)+1));
    strcpy(cliName, clientName);
    serverClientsList toSave = {
        .clientName = cliName,
        .connectedToName = NULL,
        .connectionType = connectionType,
        .confd = confd,
        .nNotResponding = 0,
        .addr = addr,
        .next = *cliList
    };
    *newEntry = toSave;

    *cliList = newEntry;
}

serverClientsList  * cliList_find(serverClientsList * cliList, char * name){
    if(cliList == NULL)
        return NULL;
    if(strcmp(cliList->clientName, name) == 0){
        return cliList;
    }
    return cliList_find(cliList->next, name);
}

void cliList_free(serverClientsList ** cliList, char * name){
    serverClientsList  * start = *cliList;
    *cliList = NULL;

    while(start != NULL){
        serverClientsList  * next = start->next;
        free(start->clientName);
        free(start);
        start = next;
    }
}

void cliList_delete(serverClientsList ** cliList, char * name){
    serverClientsList  * prev = NULL;
    serverClientsList  * curr = *cliList;
    while(curr != NULL && strcmp(curr->clientName, name) != 0){
        prev = curr;
        curr = curr->next;
    }

    if(curr == NULL)
        return;

    if(prev != NULL){
        prev->next = curr->next;
    }
    else{
        *cliList = curr->next;
    }

    free(curr->clientName);
    free(curr);
}

#endif //CW10_SERVERCLIENTS_H
