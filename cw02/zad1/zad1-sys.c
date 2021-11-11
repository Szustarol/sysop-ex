#include "zad1-functions.h"
#include <sys/stat.h>
#include <fcntl.h>

int parseFileInput(char * fileBuffer, size_t bufferSize, int fileDescriptor){
	int newLineFound = 0;
	if(fileBuffer[0] != 0x0)
		newLineFound = bufferPrintToNewline(fileBuffer, bufferSize);
	while(!newLineFound){
		size_t nRead = read(fileDescriptor, fileBuffer, bufferSize);
		fileBuffer[nRead] = 0x0;
		newLineFound = bufferPrintToNewline(fileBuffer, bufferSize);

		if(nRead < 1)
			return 1;
	}
	return 0;
}

void printout(int file1Descriptor,  int file2Descriptor){
	const size_t bufferSize = 40; //bufor przesuwny

	char file1Buffer[bufferSize+1];
	char file2Buffer[bufferSize+1];

	file1Buffer[0] = 0x0;
	file2Buffer[0] = 0x0;
	file1Buffer[bufferSize] = 0x0;
	file2Buffer[bufferSize] = 0x0;

	int file1Lock = 0, file2Lock = 0;
 
	while(file1Lock + file2Lock < 2){
		if(!file1Lock){
			file1Lock = parseFileInput(file1Buffer, bufferSize, file1Descriptor);
		}
		if(!file2Lock){
			file2Lock = parseFileInput(file2Buffer, bufferSize, file2Descriptor);
		}
	}
}

int main(int argc, char**argv){
	char ** filePaths = getFiles(argc, argv);

	int file1Descriptor = open(filePaths[0], O_RDONLY);
	int file2Descriptor = open(filePaths[1], O_RDONLY);
	
	if(file1Descriptor == -1){
		printf("File <%s> was not found or can't be accessed\n", filePaths[0]);
	}
	else if(file2Descriptor == -2){
		printf("File <%s> was not found or can't be accessed\n", filePaths[1]);
	}
	else{
		printout(file1Descriptor, file2Descriptor);
		close(file1Descriptor);
		close(file2Descriptor);
	}

	if(argc > 3){
		free(filePaths[0]); free(filePaths[1]); 
	}
	free(filePaths);
	return 0;
}