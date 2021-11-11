#ifndef DEFINES_H
#define DEFINES_H
#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

const key_t semaphoreSetKey = 0xabcd0101;
const key_t sharedMemoryKey = 0xabcd1010;

const size_t OVEN_SEMAPHORE_ID = 0;
const size_t TABLE_SEMAPHORE_ID = 1;

typedef struct{
    int nextSpotIndex;
    int spot[5];
}Oven;

typedef struct{
    int nextSpotIndex;
    int spot[5];
}Table;

union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
           };

const size_t shrMemSize = sizeof(Oven) + sizeof(Table);

int createSemaphores(){
    int smphrDesc = semget(semaphoreSetKey, 2, IPC_CREAT | IPC_EXCL | 0666);
    if(smphrDesc == -1){
        smphrDesc = semget(semaphoreSetKey, 2, 0666);
        if(smphrDesc != -1){
            semctl(smphrDesc, 0, IPC_RMID); 
            smphrDesc = semget(semaphoreSetKey, 2, IPC_CREAT | IPC_EXCL | 0666);
        }
    }
    return smphrDesc;
}

int createSharedMem(){
    int shmDesc = shmget(sharedMemoryKey, shrMemSize, IPC_CREAT | IPC_EXCL | 0666);
    if(shmDesc == -1){
        shmDesc = shmget(sharedMemoryKey, 0, 0666);
        if(shmDesc != -1){
            shmctl(shmDesc, IPC_RMID, NULL);
            shmDesc = shmget(sharedMemoryKey, shrMemSize, IPC_CREAT | IPC_EXCL | 0666);
        }
    }
    return shmDesc;
}

unsigned long getTimestamp(){
    struct timespec spec;
    clock_gettime(1, &spec);
    return spec.tv_sec*1000 + spec.tv_nsec / 1000000;
}
#endif