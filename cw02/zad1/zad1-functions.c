#include "zad1-functions.h"

static struct tms tmsStart;
static clock_t realStart;

//moves the buffer content to the next line
int bufferPrintToNewline(char * buffer, size_t bufferSize){
	for(size_t position = 0; position < bufferSize && buffer[position] != 0x0; position++){
		if(buffer[position] == '\n'){
			printf("%.*s\n", (int)position, buffer);
			//move buffer
			size_t toCopy = bufferSize - position - 1;
			memmove(buffer, buffer + position+1, toCopy);
			buffer[toCopy] = 0x0;
			return 1;
		}
	}

	printf("%s", buffer);
	if(buffer[0] == 0)
		puts("");
	buffer[0] = 0x0;

	return 0;
}

char * getInput(){	
	const size_t bufSize = 40;
	char tmp[bufSize];
	char * input = malloc(sizeof(char)*bufSize);
	size_t offset = 0;
	while(fgets(tmp, sizeof(tmp), stdin) != NULL){
		strncpy(input + offset, tmp, bufSize);
		offset += bufSize;
		input = realloc(input, bufSize + offset);
		for(size_t i = 0; i < bufSize; i++){
			if(tmp[i] == '\n'){
				input[i+offset-bufSize] = 0x0;
				goto loopBreaker;
			}
		}
	}
	loopBreaker:
	return input;
}

char ** getFiles(int argc, char ** argv){
	char ** files = malloc(sizeof(char**)*2);
	if(argc < 3){
		if(argc < 2){
			puts("Name of the first file:");
			files[0] = getInput();
		}
		else{
			files[0] = argv[1];
		}
		puts("Name of the second file:");
		files[1] = getInput();
	}
	else{
		files[0] = argv[1];
		files[1] = argv[2];
	}

	return files;
}

void timeMeasureInit(void){
	realStart = times(&tmsStart);
}

void timeMeasureCheck(void){
	struct tms tmsNow;
	clock_t realEnd = times(&tmsNow);
	printf("\tWall time: %f seconds\n\tSystem time: %f seconds\n\tUser time: %f seconds\n",
		(double)(realEnd-realStart)/sysconf(_SC_CLK_TCK),
		(double)(tmsNow.tms_stime - tmsStart.tms_stime)/sysconf(_SC_CLK_TCK),
		(double)(tmsNow.tms_utime - tmsNow.tms_utime)/sysconf(_SC_CLK_TCK));
}