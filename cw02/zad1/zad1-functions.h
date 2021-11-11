#ifndef ZAD1_FUNCTIONS_C
#define ZAD1_FUNCTIONS_C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdint.h>

//moves the buffer content to the next line
int bufferPrintToNewline(char * buffer, size_t bufferSize);

char * getInput();

char ** getFiles(int argc, char ** argv);

void timeMeasureInit(void);

void timeMeasureCheck(void);

#endif