#ifndef DEFINES_H
#define DEFINES_H
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

const char ovenSemaphoreName[] = "/pizzaHouseOvenSemaphore";
const char tableSemaphoreName[] = "/pizzaHouseTableSemaphore";
const char sharedMemName[] = "/pizzaHouseSharedMem";

sem_t * ovenSemaphore;
sem_t * tableSemaphore;

typedef struct{
    int nextSpotIndex;
    int spot[5];
}Oven;

typedef struct{
    int nextSpotIndex;
    int spot[5];
}Table;

const size_t shrMemSize = sizeof(Oven) + sizeof(Table);

sem_t * parseCreateSemaphore(const char * name){
    sem_t * p = sem_open(name, O_CREAT | O_EXCL | O_RDWR, 0666,  1);
    if(p == SEM_FAILED){
        p = sem_open(name, 0);
        if(p != SEM_FAILED){
            sem_unlink(name);
            sem_close(p);
        }
        return sem_open(name, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    }
    return p;
}

int createSemaphores(){
    ovenSemaphore = parseCreateSemaphore(ovenSemaphoreName);
    tableSemaphore = parseCreateSemaphore(tableSemaphoreName);
    if(ovenSemaphore == SEM_FAILED || tableSemaphore == SEM_FAILED)
        return -1;
    return 0;       
}

int parseCreateSharedMem(){
    int shmDesc = shm_open(sharedMemName, O_CREAT | O_EXCL | O_RDWR, 0666);
    if(shmDesc == -1){
        shmDesc = shm_open(sharedMemName, 0, 0666);
        if(shmDesc != -1){
            shm_unlink(sharedMemName);
        }
        return shm_open(sharedMemName, O_CREAT | O_EXCL | O_RDWR, 0666);
    }
    return shmDesc;
}

int createSharedMem(){
    int shmDesc = parseCreateSharedMem();
    if(shmDesc != -1){
        ftruncate(shmDesc, shrMemSize);
    }
    return shmDesc;
}

unsigned long getTimestamp(){
    struct timespec spec;
    clock_gettime(1, &spec);
    return spec.tv_sec*1000 + spec.tv_nsec / 1000000;
}
#endif