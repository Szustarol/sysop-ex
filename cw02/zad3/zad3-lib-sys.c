#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "stringchecker.h"

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

	void fileWrite(char * fileName, char * content){

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

#ifdef SYSFUN
	void saveBuffer(char * outputFileName, char * buffer, size_t bufferSize){
		int fd = open(outputFileName, O_CREAT | O_TRUNC | O_WRONLY, 0664);
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

int check(char * content, long fileSize){
	for(size_t i = 0; i < fileSize; i++){
		if(!isdigit(content[i]) && content[i] != '\n' && content[i] != 0x0 && content[i] != '-'){
			puts("Non-digit data detected in source file!");
			return 0;
		}
	}
	return 1;
}

void parseA(char ** linePointers, size_t * lineLengths, size_t nLines){
	size_t np = 0;
	for(size_t i = 0; i < nLines; i++){
		if(lineLengths[i] > 0
			&& ((*(linePointers[i] + lineLengths[i]-1) - '0') % 2 == 0)){
			np++;
		}
	}

	char * output = malloc(sizeof(char)*100);
	sprintf(output, "Liczb parzystych jest %lu\n", np);
	saveBuffer("a.txt", output, strlen(output));
	free(output);
}

void parseB(char ** linePointers, size_t * lineLengths, size_t nLines){
	size_t lenValid = 0;
	for(size_t i = 0; i < nLines; i++){
		if(lineLengths[i] > 1){
			size_t tenDig = *(linePointers[i] + lineLengths[i]-2) - '0';
			if(tenDig == 7 || tenDig == 0){
				lenValid += lineLengths[i]+1;
			}
		}
	}

	char * output = calloc(sizeof(char), lenValid+1);
	size_t offset = 0;
	for(size_t i = 0; i < nLines; i++){
		if(lineLengths[i] > 1){
			size_t tenDig = *(linePointers[i] + lineLengths[i]-2) - '0';
			if(tenDig == 7 || tenDig == 0){
				memcpy(output+offset, linePointers[i], lineLengths[i]);
				output[offset+lineLengths[i]] = '\n';
				offset += lineLengths[i]+1;
			}
		}
	}

	saveBuffer("b.txt", output, strlen(output));
	free(output);
}

void parseC(char ** linePointers, size_t * lineLengths, size_t nLines){
	char * output = NULL;
	size_t osize = 0;
	for(size_t i = 0; i < nLines; i++){
		char * value = calloc(sizeof(char), lineLengths[i]+1);
		memcpy(value, linePointers[i], lineLengths[i]);
		char * lineSqrt = digitSqrt(value);
		char * sqrtMultip = digitStringMultiply(lineSqrt, lineSqrt);
		if(compare(value, sqrtMultip) == EQUAL){
			output = realloc(output, osize + lineLengths[i]+1);
			memcpy(output + osize, linePointers[i], lineLengths[i]);
			output[osize+lineLengths[i]] = '\n';
			osize += lineLengths[i] + 1;
		}
		free(lineSqrt);
		free(sqrtMultip);
		free(value);
	}
	saveBuffer("c.txt", output, strlen(output));
	free(output);
}

void parseContent(char * content, long fileSize, char *** linePointers, size_t ** lineLengths, size_t * nl){
	size_t nLines = 1;
	for(size_t i = 0; i < fileSize; i++){
		if(content[i] == '\n')
			nLines++; 
	}

	*linePointers = calloc(sizeof(char*), nLines);
	*lineLengths = calloc(sizeof(size_t), nLines);
	size_t offset = 0;
	size_t index = 0;
	for(size_t i = 0; i < fileSize; i++){
		if(content[i] == '\n'){
			(*linePointers)[index] = content + offset;
			(*lineLengths)[index] = i-offset;
			offset = i+1;
			index++;
		}
	}
	(*linePointers)[index] = content + offset;
	(*lineLengths)[index] = fileSize-offset;

	*nl = nLines;
}

int main(int argc, char ** argv){
	long fileSize;
	char * content = loadFile("dane.txt", &fileSize);
	if(content == NULL){
		puts("No input file found (dane.txt).");
		return 0;
	}
	char ** linePointers;
	size_t * lineLengths;
	size_t nLines;
	parseContent(content, fileSize, &linePointers, &lineLengths, &nLines);
	parseA(linePointers, lineLengths, nLines);
	parseB(linePointers, lineLengths, nLines);
	parseC(linePointers, lineLengths, nLines);

	return 0;
}