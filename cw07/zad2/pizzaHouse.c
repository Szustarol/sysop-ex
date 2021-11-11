#include "defines.h"

pid_t * children = NULL;   
size_t childIndex = 0;

int sharedMemDesc;

sem_t * ovenSemaphore = SEM_FAILED;;
sem_t * tableSemaphore = SEM_FAILED;

Oven * oven;
Table * table;


void houseClosing(int s){
    for(size_t i = 0; i < childIndex; i++){
        kill(children[i], SIGINT);
    }
}

//WORKERS
bool keepWorking = true;

void intHandler(int signo){
    munmap(oven, shrMemSize);
    sem_close(ovenSemaphore);
    sem_close(tableSemaphore);
    exit(0);
}


void cookLoop(void){
    puts("Cook starting.");
    signal(SIGINT, intHandler);
    pid_t myPid = getpid();
    srand(time(NULL) ^ (myPid << 8));
    while(keepWorking){
        int pizzaType = rand() % 10;
        int sleepDelay = (rand() % 1000 + 1000)* 1000;
        usleep(sleepDelay);
        printf("(%d %lu) Przygotowuje pizze: %d\n", myPid, getTimestamp(), pizzaType);
        bool inserted = false;
        int nInOven = 0;
        int atPos = 0;
        while(!inserted){
            sem_wait(ovenSemaphore);
            if(oven->spot[oven->nextSpotIndex] == -1){
                oven->spot[oven->nextSpotIndex] = pizzaType;
                atPos = oven->nextSpotIndex;
                oven->nextSpotIndex = (oven->nextSpotIndex+1) % 5;
                inserted = true;
                nInOven = 0;
                for(size_t i = 0; i < 5; i++){
                    if(oven->spot[i] != -1){
                        nInOven ++;
                    }
                }
            }
            sem_post(ovenSemaphore);
        }
        printf("(%d %lu) Dodalem pizze: %d. Liczba pizz w piecu: %d.\n", myPid, getTimestamp(), pizzaType, nInOven);
        sleepDelay = (rand() % 3000 + 1000) * 1000;
        usleep(sleepDelay);
        inserted = false;
        int nOnTable = -1;
        while(!inserted){
            sem_wait(tableSemaphore);
            if(table->spot[table->nextSpotIndex] == -1){
                sem_wait(ovenSemaphore);
                oven->spot[atPos] = -1;
                nInOven = 0;
                for(size_t i = 0; i < 5; i++){
                    if(oven->spot[i] == -1){
                        nInOven++;
                    }
                }
                sem_post(ovenSemaphore);
                inserted = true;
                nOnTable = 0;
                table->spot[table->nextSpotIndex] = pizzaType;
                table->nextSpotIndex = (table->nextSpotIndex + 1) % 5;
                for(size_t i = 0; i < 5; i++){
                    if(table->spot[i] != -1){
                        nOnTable++;
                    }
                }
                
            }
            sem_post(tableSemaphore);        
        }
        printf("(%d %lu) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n", 
        myPid, getTimestamp(), pizzaType, nInOven, nOnTable);

    }
    intHandler(-1);
}

void driverLoop(void){
    puts("Driver starting");
    signal(SIGINT, intHandler);
    pid_t myPid = getpid();
    srand(time(NULL) ^ (myPid << 8));
    while(keepWorking){
        int nSpots = 0;
        int take = -1;
        int pizzaType = -1;
        sem_wait(tableSemaphore);
        for(int i = 0; i < 5; i++){
            if(table->spot[i] != -1){
                take = i;
                nSpots++;
            }
        }
        if(take != -1){
            printf("(%d %lu) Pomieram pizze: %d. Liczba pizz na stole: %d.\n", 
                myPid, getTimestamp(), table->spot[take], nSpots-1);
            pizzaType = table->spot[take];
            table->spot[take] = -1;            
        }
        sem_post(tableSemaphore);
        if(take != -1){
            int sleepDelay = (rand() % 4000 + 1000)* 1000;
            usleep(sleepDelay);     
            printf("(%d %lu) Dostarczam pizze: %d.\n", myPid, getTimestamp(), pizzaType);       
            sleepDelay = (rand() % 4000 + 1000)* 1000;
            usleep(sleepDelay);
        }

    }
    printf("Driver (pid %d) ends working\n", myPid);
    intHandler(-1);
}

int main(int argc, char ** argv){
    if(argc < 3){
        puts("Usage: program [n_cooks] [n_drivers]");
        return 0;
    }
    int nCooks = atoi(argv[1]);
    int nDrivers = atoi(argv[2]);
    createSemaphores();
    sharedMemDesc = createSharedMem();
    if(ovenSemaphore == SEM_FAILED || tableSemaphore == SEM_FAILED){
        puts("Unable to create semaphores");
        return 0;
    }
    if(sharedMemDesc == -1){
        puts("Unable to create shared memory");
        return 0;
    }

    puts(strerror(errno));
    oven = mmap(NULL, shrMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemDesc, 0);
    puts(strerror(errno));

    table = (Table*)((void*)oven + sizeof(Oven));

    for(size_t i = 0; i < 5; i++){
        oven->spot[i] = -1;
        table->spot[i] = -1;
    }
    oven->nextSpotIndex = 0;
    table->nextSpotIndex = 0;

    if(nCooks < 1 || nDrivers < 1){
        puts("Invalid number of workers.");
        return 0;
    }
    pid_t chPid = 0;
    for(size_t i = 0; i < nCooks; i++){
        if((chPid = fork()) == 0){
            cookLoop();
            return 0;
        }
        else{
            children = realloc(children, sizeof(pid_t)*++childIndex);
            children[childIndex-1] = chPid;
        }
    }

    for(size_t i = 0; i < nDrivers; i++){
        if((chPid = fork()) == 0){
            driverLoop();
            return 0;
        }
        else{
            children = realloc(children, sizeof(pid_t)*++childIndex);
            children[childIndex-1] = chPid;
        }        
    }

    signal(SIGINT, houseClosing);

    for(size_t i = 0; i < childIndex; i++){
        waitpid(children[i], NULL, 0);
    }

    free(children);
    munmap(oven, shrMemSize);
    shm_unlink(sharedMemName);
    sem_close(ovenSemaphore);
    sem_close(tableSemaphore);
    sem_unlink(tableSemaphoreName);
    sem_unlink(ovenSemaphoreName);
    return 0;
}