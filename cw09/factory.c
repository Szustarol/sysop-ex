#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct{
    bool shouldSantaWakeUp;
    pthread_cond_t santaWakeUpCond;
    pthread_mutex_t santaWakeUpMutex;
    
    int nReindeersWaiting;
    pthread_mutex_t reindeersWaitingMutex;
    int reindeersToGo[9];
    pthread_cond_t reindeersGoCond;
    pthread_mutex_t reindeersGoMutex;

    int elvesWaiting[3];
    pthread_mutex_t elvesWaitingMutex;
    pthread_cond_t elvesWaitingCond;
    int elfToGo[10];
    pthread_cond_t elvesGoCond;
    pthread_mutex_t elvesGoMutex;

}FactoryData;

void * elfLoop(void * data){
    FactoryData * factoryData = data;
    int myId;
    //for elf ID initialisation
    pthread_mutex_lock(&factoryData->elvesGoMutex);
    for(int i = 0; i < 10; i++){
        if(factoryData->elfToGo[i] == false){
            myId = i;
            factoryData->elfToGo[i] = true;
            break;
        }
    }
    if(myId == 9){
        for(int i = 0; i < 10; i++){
            factoryData->elfToGo[i] = false;
        }
        pthread_cond_broadcast(&factoryData->elvesGoCond);
    }
    pthread_mutex_unlock(&factoryData->elvesGoMutex);

    pthread_mutex_lock(&factoryData->elvesGoMutex);
    while(factoryData->elfToGo[myId] == true){
        pthread_cond_wait(&factoryData->elvesGoCond, &factoryData->elvesGoMutex);
    }
    pthread_mutex_unlock(&factoryData->elvesGoMutex);


    while(true){
        usleep((rand() % 3000 + 2000)*1000);
        bool takenPosition = false;
        pthread_mutex_lock(&factoryData->elvesWaitingMutex);
        while(!takenPosition){
            for(int i = 0; i < 3; i++){
                if(factoryData->elvesWaiting[i] == -1){
                    factoryData->elvesWaiting[i] = myId;
                    takenPosition = true;
                    printf("Elf: czeka %d elfów na mikołaja, %d\n", i+1, myId);
                    if(i == 2){
                        printf("Elf: wybudzam Mikołaja, %d\n", myId);
                        pthread_mutex_lock(&factoryData->santaWakeUpMutex);
                        factoryData->shouldSantaWakeUp = true;
                        pthread_mutex_unlock(&factoryData->santaWakeUpMutex);
                        pthread_cond_signal(&factoryData->santaWakeUpCond);
                    }
                    break;
                }
            }
            if(!takenPosition){
                printf("Elf: czeka na powrót elfów, %d\n", myId);
                pthread_cond_wait(&factoryData->elvesWaitingCond, &factoryData->elvesWaitingMutex);
            }
        }
        pthread_mutex_unlock(&factoryData->elvesWaitingMutex);    

        pthread_mutex_lock(&factoryData->elvesGoMutex);
        while(!factoryData->elfToGo[myId]){
            pthread_cond_wait(&factoryData->elvesGoCond, &factoryData->elvesGoMutex);
        }
        factoryData->elfToGo[myId] = false;
        printf("Elf: Mikołaj rozwiązuje problem, %d\n", myId);
        pthread_mutex_unlock(&factoryData->elvesGoMutex);  
        usleep((rand() % 1000 + 1000)*1000);
    }

    return NULL;
}

void * reindeerLoop(void * data){
    FactoryData * factoryData = data;
    int myQueueID = -1;
    while(true){
        usleep((rand() % 5000 + 5000)*1000);

        pthread_mutex_lock(&factoryData->reindeersWaitingMutex);
        if(myQueueID == -1){
            myQueueID = factoryData->nReindeersWaiting;
        }
        factoryData->nReindeersWaiting++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", factoryData->nReindeersWaiting, myQueueID);
        if(factoryData->nReindeersWaiting == 9){
            printf("Renifer: wybudzam Mikołaja, %d\n", myQueueID);
            pthread_mutex_lock(&factoryData->santaWakeUpMutex);
            factoryData->shouldSantaWakeUp = true;
            pthread_mutex_unlock(&factoryData->santaWakeUpMutex);
            pthread_cond_signal(&factoryData->santaWakeUpCond);
        }
        pthread_mutex_unlock(&factoryData->reindeersWaitingMutex);

        usleep((rand() % 2000 + 2000)*1000);

        pthread_mutex_lock(&factoryData->reindeersGoMutex);
        while(factoryData->reindeersToGo[myQueueID] == false){
            pthread_cond_wait(&factoryData->reindeersGoCond, &factoryData->reindeersGoMutex);
        }
        factoryData->reindeersToGo[myQueueID] = false;
        pthread_mutex_unlock(&factoryData->reindeersGoMutex);

    }

    return NULL;
}

void * santaLoop(void * data){
    FactoryData * factoryData = data;
    while(true){
        pthread_mutex_lock(&factoryData->santaWakeUpMutex);

        while(!factoryData->shouldSantaWakeUp){
            pthread_cond_wait(&factoryData->santaWakeUpCond, &factoryData->santaWakeUpMutex);
        }

        factoryData->shouldSantaWakeUp = false;

        pthread_mutex_unlock(&factoryData->santaWakeUpMutex);


        bool deliverToys = false;

        pthread_mutex_lock(&factoryData->reindeersWaitingMutex);
        if(factoryData->nReindeersWaiting == 9){
            factoryData->nReindeersWaiting = 0;
            pthread_mutex_lock(&factoryData->reindeersGoMutex);
            for(int i = 0; i < 9; i++){
                factoryData->reindeersToGo[i] = true;
            }
            pthread_mutex_unlock(&factoryData->reindeersGoMutex);
            pthread_cond_broadcast(&factoryData->reindeersGoCond);
            deliverToys = true;
        }
        pthread_mutex_unlock(&factoryData->reindeersWaitingMutex);

        if(deliverToys){
            puts("Mikołaj: dostarczam zabawki");
            usleep((rand() % 2000 + 2000)*1000);
        }

        pthread_mutex_lock(&factoryData->elvesWaitingMutex);

        bool threeWaiting = true;
        for(int i = 0; i < 3; i++){
            if(factoryData->elvesWaiting[i] == -1)
                threeWaiting = false;
        }
        pthread_mutex_unlock(&factoryData->elvesWaitingMutex);


        if(threeWaiting){
            pthread_mutex_lock(&factoryData->elvesGoMutex);
            pthread_mutex_lock(&factoryData->elvesWaitingMutex);
            printf("Mikołaj: rozwiązuję problemy elfów %d %d %d\n",
                factoryData->elvesWaiting[0],
                factoryData->elvesWaiting[1],
                factoryData->elvesWaiting[2]);
            for(int i = 0; i < 3; i++){
                factoryData->elfToGo[factoryData->elvesWaiting[i]] = true;
            }
            pthread_cond_broadcast(&factoryData->elvesGoCond);
            pthread_mutex_unlock(&factoryData->elvesGoMutex);
            usleep((rand() % 1000 + 1000)*1000);
            for(int i = 0; i < 3; i++){
                factoryData->elvesWaiting[i] = -1;
            }
            pthread_cond_broadcast(&factoryData->elvesWaitingCond);
            pthread_mutex_unlock(&factoryData->elvesWaitingMutex);
        }


        puts("Mikołaj: zasypiam");


    }

    return NULL;
}

int main(int argc, char ** argv){
    srand(time(NULL));

    FactoryData fd = {
        .shouldSantaWakeUp = false,
        .santaWakeUpCond = PTHREAD_COND_INITIALIZER,
        .santaWakeUpMutex = PTHREAD_MUTEX_INITIALIZER,

        .nReindeersWaiting = 0,
        .reindeersWaitingMutex = PTHREAD_MUTEX_INITIALIZER,
        .reindeersGoCond = PTHREAD_COND_INITIALIZER,
        .reindeersToGo = {false},
        .reindeersGoMutex = PTHREAD_MUTEX_INITIALIZER,
        
        .elvesWaiting = {-1, -1, -1},
        .elvesWaitingMutex = PTHREAD_MUTEX_INITIALIZER,
        .elvesWaitingCond = PTHREAD_COND_INITIALIZER,
        .elfToGo = {false},
        .elvesGoMutex = PTHREAD_MUTEX_INITIALIZER,
        .elvesGoCond = PTHREAD_COND_INITIALIZER
    };

    pthread_t santa;
    pthread_t reindeers[9];
    pthread_t elves[10];

    pthread_create(&santa, NULL, santaLoop, &fd);
    for(int i = 0; i < 9; i++){
        pthread_create(reindeers+i, NULL, reindeerLoop, &fd);
    }
    for(int i = 0; i < 10; i++){
        pthread_create(elves+i, NULL, elfLoop, &fd);
    }


    pthread_join(santa, NULL);
    for(int i = 0; i < 9; i++){
        pthread_join(reindeers[i], NULL);
    }
    for(int i = 0; i < 10; i++){
        pthread_join(elves[i], NULL);
    }
    

    return 0;
}