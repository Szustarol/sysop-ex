#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#ifdef SYSFUN
	char * loadFile(char * fileName, long * fileSize){
		int fd;
		if((fd = open(fileName, O_RDONLY)) == -1)
			return NULL;
		*fileSize = lseek(fd, 0UL, SEEK_END);
		lseek(fd, 0UL, SEEK_SET);
		char * buffer = malloc(sizeof(char) * (*fileSize));
		read(fd, buffer, *fileSize);
		close(fd);
		return buffer;
	}
#else
	char * loadFile(char * fileName, long * fileSize){
		FILE * file;
		if((file =  fopen(fileName, "r")) == NULL)
			return NULL;
		char * buffer;
		fseek(file, 0UL, SEEK_END);
		*fileSize = ftell(file);
		buffer = malloc(sizeof(char)*(*fileSize));
		fseek(file, 0UL, SEEK_SET);
		fread(buffer, sizeof(char), *fileSize, file);
		fclose(file);
		return buffer;
	}
#endif



void printContaining(char * fileBuffer, char character, long fileSize){
	// find all newlines
	size_t nNewlines = 1;
	for(size_t i = 0; fileBuffer[i] != 0x0; i++)
		if(fileBuffer[i] == '\n') nNewlines++;

	//allocate buffers
	char ** lines = malloc(sizeof(char*)* nNewlines);
	size_t * lineLengths = malloc(sizeof(size_t)* nNewlines);
	uint8_t * containsCharacter = calloc(sizeof(uint8_t), nNewlines);
	size_t position = 0;
	for(size_t i = 0, j = 0; i < fileSize; i++){
		if(fileBuffer[i] == character){
			containsCharacter[j] = 1;
		}
		else if(fileBuffer[i] == '\n'){
			lines[j] = fileBuffer + position;
			lineLengths[j] = i-position;
			position = i+1;
			j++;
			fileBuffer[i] = 0x0; // for strstr
		}
	}
	//add the last line
	lines[nNewlines-1] = fileBuffer + position;

	for(size_t i = 0; i < nNewlines; i++){
		if(containsCharacter[i]){
			printf("%s\n", lines[i]);
		}
	}

	free(lines);
	free(lineLengths);
	free(containsCharacter);
}

int main(int argc, char ** argv){
	if (argc < 3){
		puts("Invalid parameters. Usage: program [character] [file]");
		return 0;
	}
	if(strlen(argv[1]) != 1){
		puts("The second parameter should be just a character");
		return 0;
	}

	long fileSize;
	char * fileBuffer = loadFile(argv[2], &fileSize);

	if(fileBuffer == NULL){
		puts("The requested file couldn't be opened");
		return 0;
	}

	printContaining(fileBuffer, argv[1][0], fileSize);
	free(fileBuffer);

	return 0;
}