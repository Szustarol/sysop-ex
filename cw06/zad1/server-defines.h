#ifndef SERVER_DEFINES_H
#define SERVER_DEFINES_H
#include "defines.h"

typedef struct client__list{
    struct client__list * next;
    int clientID;
    int queueID;
    int connected;
}clientList;

void clientListFree(clientList * list){
    if(list != NULL){
        clientList * next = list->next;
        free(list);
        clientListFree(next);
    }
}

void clientListInsert(clientList ** list, int ID, int queueID){
    clientList * nl = calloc(1, sizeof(clientList));
    nl->next = *list;
    *list = nl;
    nl->queueID = queueID;
    nl->clientID = ID;
    nl->connected = 0;
}

clientList * clientListFind(clientList * list, int ID){
    if(list == NULL)
        return list;
    if(ID == list->clientID)
        return list;
    return clientListFind(list->next, ID);
}

void clientListRemove(clientList ** list, int ID){
    clientList * hd = *list;
    clientList * prev = NULL;
    while(hd != NULL){
        if (hd->clientID == ID){
            if(hd == *list){
                *list = hd->next;
                free(hd);
            }
            else{
                prev->next = hd->next;
                free(hd);
            }
            return;
        }
        prev = hd;
        hd = hd->next;
    }
}
#endif