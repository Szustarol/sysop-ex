#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>


size_t * kmpCompile(char * pattern){
	size_t len = strlen(pattern);
	size_t * kmpTable = calloc(sizeof(size_t), len);
	size_t j = 0;
	for(size_t index = 1; index < len; index++){
		while(j > 0 && pattern[index] != pattern[j])
			j = kmpTable[j-1];
		if(pattern[index] == pattern[j]){
			j++;
		}
		kmpTable[index] = j;
	}
	return kmpTable;
}

#define MAX(a, b) a > b ? a : b

char * replace(char * buffer, char * pattern, char * replacement, size_t bufferSize){
	size_t patLen = strlen(pattern);
	size_t repLen = strlen(replacement);
	//compute maximum output size
	size_t outputMaxSize = MAX((bufferSize/patLen+1)*repLen, bufferSize)+1;

	size_t * table = kmpCompile(pattern);
	char * output = calloc(sizeof(char), outputMaxSize);
	size_t kmpIndex = 0;
	size_t outputOffset = 0;

	for(size_t i = 0; i < bufferSize; i++){
		while(kmpIndex > 0 && buffer[i] != pattern[kmpIndex]){
			kmpIndex = table[kmpIndex-1];
		}

		if(buffer[i] == pattern[kmpIndex]){
			kmpIndex+=1;
		}

		if(kmpIndex == patLen){
			//since we are replacing - reset kmp search
			kmpIndex = 0;
			//rolback replacement len characters-1 (since last char from pattern isn't added)
			outputOffset -= patLen-1;
			memcpy(output + outputOffset, replacement, repLen*sizeof(char));
			outputOffset += repLen;
		}
		else{
			output[outputOffset] = buffer[i];
			outputOffset++;
		}
	}
	output[outputOffset] = 0x0;

	free(table);
	return output;
}	

#undef MAX

#ifdef SYSFUN
	char * getFileContent(char * path, size_t * fileSize){
		int fd = open(path, O_RDONLY);
		if(fd == -1)
			return NULL;
		*fileSize = lseek(fd, 0UL, SEEK_END);
		lseek(fd, 0UL, SEEK_SET);
		char * buffer = calloc(sizeof(char), (*fileSize)+1);
		read(fd, buffer, *fileSize);
		close(fd);
		return buffer;
	}

#else
	char * getFileContent(char * path, size_t * fileSize){
		FILE * file = fopen(path, "r");
		if(file == NULL)
			return NULL;
		fseek(file, 0UL, SEEK_END);
		*fileSize = ftell(file);
		fseek(file, 0UL, SEEK_SET);
		char * buffer = calloc(sizeof(char), (*fileSize)+1);
		fread(buffer, sizeof(char), *fileSize, file);
		fclose(file);
		return buffer;
	}
#endif

#ifdef SYSFUN
	void saveBuffer(char * outputFileName, char * buffer, size_t bufferSize){
		int fd = open(outputFileName, O_CREAT | O_TRUNC | O_WRONLY);
		if(fd != -1){
			write(fd, buffer, sizeof(char) * bufferSize);
			close(fd);
		}
	}
#else
	void saveBuffer(char * outputFileName, char * buffer, size_t bufferSize){
		FILE * file = fopen(outputFileName, "w+");
		if(file){
			fwrite(buffer, sizeof(char), bufferSize, file);
			fclose(file);
		}
	}
#endif

int main(int argc, char ** argv){
	char *inputBuffer;
	size_t inputSize;
	if(argc < 5){
		puts("Usage: program <source file> <target file> <pattern> <replacement>");
	}
	else{
		inputBuffer = getFileContent(argv[1], &inputSize);
		if(inputBuffer == NULL){
			printf("Couldn't locate the source file (%s).\n", argv[1]);
			return 0;
		}
		char * output = replace(inputBuffer, argv[3], argv[4], inputSize);
		saveBuffer(argv[2], output, strlen(output));
		free(inputBuffer);

	}
	return 0;
}