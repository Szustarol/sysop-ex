#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <wait.h>
#include <limits.h>

int nDigits(int number){
    int sum = 0;
    while(number > 0){
        sum++;
        number = number / 10;
    }
    return sum;
}

void producerLoop(int myNumber, int charactersPerRead, char * fifoPath, char * inputPath){
    //open the input file
    FILE * input = fopen(inputPath, "r");
    if(!input){
        printf("Unable to open %s\n", inputPath);
        return;
    }

    FILE * pipe = fopen(fifoPath, "w");
    if(!pipe){
        printf("Unable to open pipe %s\n", fifoPath);
        fclose(input);
        return;
    }

    int numberLen = nDigits(myNumber);
    int readOffset = numberLen + 1;

    char * wrBuffer = calloc(readOffset + charactersPerRead + 2, sizeof(char));
    wrBuffer[readOffset + charactersPerRead + 1] = 0;
    sprintf(wrBuffer, "%d:", myNumber);

    char * rdBuffer = wrBuffer + readOffset;

    int nRead;

    setvbuf(pipe, NULL, _IONBF, 0);

    do{
        int msDelay = (rand() % 15) + 5;
        usleep(msDelay*1000);
        nRead = fread(rdBuffer, sizeof(char), charactersPerRead, input);
        //if number of characters read is not equal charactersPerRead then It will be padded with zeros
        //also all newlines will be replaced by spaces
        for(size_t i = nRead; i < charactersPerRead; i++){
            rdBuffer[i] = 0;
        }
        for(size_t i = 0; i < nRead; i++){
            if(rdBuffer[i] == '\n')
                rdBuffer[i] = ' ';
        }
        fwrite(wrBuffer, sizeof(char), readOffset + charactersPerRead, pipe);

    }while(nRead > 0);

    free(wrBuffer);
    fclose(input);
}

int main(int argc, char ** argv){
    srand(time(NULL));

    if(argc < 5){
        puts("Usage: program [FIFO path] [producer number] [text file path] [number of characters per read]");
        return 0;
    }
    int myNumber;
    int charactersPerRead;
    char * fifoPath = argv[1];
    char * inputPath = argv[3];

    if((myNumber = atoi(argv[2])) < 1){
        puts("Invalid producer number - must be an integer greater than 0");
        return 0;
    }

    if((charactersPerRead = atoi(argv[4])) < 1){
        puts("Invalid number of characters per read. Must be an integer greater than 0");
        return 0;
    }

    producerLoop(myNumber, charactersPerRead, fifoPath, inputPath);

    return 0;    
}
