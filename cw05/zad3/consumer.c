#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>
#include <wait.h>
#include <stdbool.h>
#include <limits.h>

#define MIN(a, b) a > b ? b : a

void fileAppend(FILE * fileHandle, char * text, size_t lineNumber){
    int fNum = fileno(fileHandle);
    flock(fNum, LOCK_EX);
    size_t nLines = 1;
    char c;
    fseek(fileHandle, 0, SEEK_SET);
    int before = -1;
    while(fread(&c, sizeof(char), 1, fileHandle) > 0){
        if((c == '\n' || c == 0x0) && lineNumber == nLines){//append right there
            before = ftell(fileHandle)-1;
        }
        if(c == '\n')
            nLines++;
    }

    size_t fileSize = ftell(fileHandle);
    if(before != -1){
        fseek(fileHandle, before, SEEK_SET);
        char * contentAfter = malloc(sizeof(char)*(fileSize - before));
        fread(contentAfter, sizeof(char), fileSize - before, fileHandle);
        fseek(fileHandle, before, SEEK_SET);
        fwrite(text, sizeof(char), strlen(text), fileHandle);
        fwrite(contentAfter, sizeof(char), fileSize - before, fileHandle);
        free(contentAfter);
    }
    else{
        while(lineNumber != nLines){
            fwrite("\n", sizeof(char), 1, fileHandle);
            nLines++;
        }
        fwrite(text, sizeof(char), strlen(text), fileHandle);
    }
    fseek(fileHandle, 0, SEEK_SET);
    flock(fNum, LOCK_UN);
}

void consumerLoop(FILE * fifoHandle, FILE * outputHandle, int numberOfCharactersRead){
    setvbuf(fifoHandle, NULL, _IONBF, 0);
    int fifoDesc = fileno(fifoHandle);
    int incomingFrom;
    int result;
    while(flock(fifoDesc, LOCK_EX) == 0 && (result = fscanf(fifoHandle, "%d:", &incomingFrom)) >= 0){
        if(result == 0){
            flock(fifoDesc, LOCK_UN);
            continue;
        }
        //read input from file
        char buffer[numberOfCharactersRead+1];
        buffer[numberOfCharactersRead] = 0;
        size_t readLen = 0;
        while(readLen < numberOfCharactersRead){
            readLen += fread(buffer + readLen, sizeof(char), numberOfCharactersRead-readLen, fifoHandle);
        }
        buffer[readLen] = 0;   
        flock(fifoDesc, LOCK_UN);
        fileAppend(outputHandle, buffer, incomingFrom);

    }
}
 
int main(int argc, char ** argv){
    if(argc < 4){
        puts("Usage: program [FIFO path] [output file path] [number of characters per read]");
    }

    int numberOfCharactersRead;

    if((numberOfCharactersRead = atoi(argv[3]))  < 1){
        puts("Number of characters per read must be greater than 0");
        return 0;
    }

    FILE * fifoHandle, * outputHandle;

    if(!(fifoHandle = fopen(argv[1], "r"))){
        printf("Unable to open FIFO %s\n", argv[1]);
        return 0;
    }

    if(!(outputHandle = fopen(argv[2], "w+r"))){
        printf("Unable to open output file %s\n", argv[2]);
        fclose(fifoHandle);
        return 0;
    }

    consumerLoop(fifoHandle, outputHandle, numberOfCharactersRead);

    fclose(fifoHandle);
    fclose(outputHandle);
    return 0;
}
