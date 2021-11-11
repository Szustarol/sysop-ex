#include "zad1-functions.h"


int parseFileInput(char * fileBuffer, size_t bufferSize, FILE * file){
	int newLineFound = 0;
	if(fileBuffer[0] != 0x0)
		newLineFound = bufferPrintToNewline(fileBuffer, bufferSize);
	while(!newLineFound){
		size_t nRead = fread(fileBuffer, sizeof(char), bufferSize, file);
		fileBuffer[nRead] = 0x0;
		newLineFound = bufferPrintToNewline(fileBuffer, bufferSize);

		if(nRead < 1)
			return 1;
	}
	return 0;
}

void printout(FILE * file1, FILE * file2){
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
			file1Lock = parseFileInput(file1Buffer, bufferSize, file1);
		}
		if(!file2Lock){
			file2Lock = parseFileInput(file2Buffer, bufferSize, file2);
		}
	}
}

int main(int argc, char**argv){
	char ** filePaths = getFiles(argc, argv);

	FILE * file1 = fopen(filePaths[0], "r");
	FILE * file2 = fopen(filePaths[1], "r");

	if(file1 == NULL){
		printf("File <%s> was not found or can't be accessed\n", filePaths[0]);
	}
	else if(file2 == NULL){
		printf("File <%s> was not found or can't be accessed\n", filePaths[1]);
	}
	else{
		printout(file1, file2);
		fclose(file1);
		fclose(file2);
	}

	if(argc > 3){
		free(filePaths[0]); free(filePaths[1]); 
	}
	free(filePaths);
	return 0;
}