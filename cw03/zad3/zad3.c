#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

#define MAX(a, b) a > b ? a : b

size_t * kmpCompile(char * pattern){
	size_t patLen = strlen(pattern);
	size_t * t = calloc(sizeof(size_t), strlen(pattern));

	size_t j = 0;
	for(size_t i = 1; i < patLen; i++){
		while(j > 0 && pattern[i] != pattern[j]){
			j = t[j-1];
		}

		if(pattern[j] == pattern[i]){
			j += 1;
		}
		t[i] = j;
	}
	return t;
}

int isStringPresentInFile(char * filePath, char * string){
	FILE * file = fopen(filePath, "r");
	if(!file)
		return 0;

	size_t patLen = strlen(string);
	size_t * kmpTable = kmpCompile(string);
	size_t bufferSize = MAX(patLen+2, 40);
	char * buffer = malloc(sizeof(char)* bufferSize);

	size_t nRead = 0;
	size_t j = 0;

	while((nRead = fread(buffer, sizeof(char), bufferSize, file)) != 0){
		for(size_t i = 0; i < nRead; i++){
			while(j > 0 && buffer[i] != string[j]){
				j = kmpTable[j-1];
			}

			if(buffer[i] == string[j])
				j += 1;

			if(j == patLen){
				free(kmpTable);
				free(buffer);
				fclose(file);
				return 1;
			}
		}
	}

	free(kmpTable);
	free(buffer);
	fclose(file);
	return 0;
}

int isFileText(char * filePath){
	const size_t testBufferSize = 4096;
	FILE * file;
	file = fopen(filePath, "r");
	if(file == NULL)
		return 0;
	char buf[testBufferSize];//just as diff does it
	size_t nread = fread(buf, sizeof(char), testBufferSize, file);
	fclose(file);
	for(size_t i = 0; i < nread-1; i++){
		if(!isprint(buf[i]) && !isspace(buf[i]))
			return 0;
	}
	return 1;
}

void pathJoin(char ** path, const char * newDirectory){//replaces the input buffer
	size_t pathLen = strlen(*path);
	size_t dirLen = strlen(newDirectory);
	if((*path)[pathLen-1] == '/'){ //skip the slash?
		if(newDirectory[0] == '/'){// skip the slash.
			*path = realloc(*path, sizeof(char)*(pathLen+dirLen));
			(*path)[pathLen+dirLen-1] = 0x0;
			strncpy(*path + pathLen, newDirectory+1, dirLen-1);
		}
		else{
			*path = realloc(*path, sizeof(char)*(pathLen+dirLen+1));
			(*path)[pathLen+dirLen] = 0x0;
			strncpy(*path + pathLen, newDirectory, dirLen);	
		}
	}
	else{//add a slash?
		if(newDirectory[0] == '/'){// nope.
			*path = realloc(*path, sizeof(char)*(pathLen+dirLen+1));
			(*path)[pathLen+dirLen] = 0x0;
			strncpy(*path + pathLen, newDirectory, dirLen);
		}
		else{
			*path = realloc(*path, sizeof(char)*(pathLen+dirLen+2));
			
			(*path)[pathLen+dirLen+1] = 0x0;
			strncpy(*path + pathLen+1, newDirectory, dirLen);	
			(*path)[pathLen] = '/';
		}	
	}
}

int main(int argc, char ** argv){
	pid_t myPid = getpid();
	size_t initDepth;
	//argv[1] - directory
	//argv[2] - string to search for
	//argv[3] - maximum depth
	if(argc < 4){
		puts("Usage: program [search_start_directory] [pattern] [maximum search depth]");
		return 0;
	}

	if((initDepth = atoi(argv[3])) == 0){
		puts("Invalid depth specified");
		return 0;
	}


	DIR * directory = opendir(argv[1]);
	if(!directory){
		printf("Unable to open directory %s\n", argv[1]);
		return 0;
	}

    struct dirent * dirEntry;
    pid_t * childrenPids = NULL;
    size_t nChildren = 0;

	while((dirEntry = readdir(directory)) != NULL){
		//is this file possibly a directory?
		char * name = malloc(sizeof(char)*(strlen(argv[1])+1));
		name[strlen(argv[1])] = 0;
		strcpy(name, argv[1]);

		if(strcmp(dirEntry->d_name, ".") == 0){
			continue;
		}
		if(strcmp(dirEntry->d_name, "..") == 0){
			continue;
		}
		pathJoin(&name, dirEntry->d_name);
		struct stat nameStat;
		stat(name, &nameStat);
		if(S_ISDIR(nameStat.st_mode)){//a directory
			if(initDepth > 1){
				pid_t childPid = fork();
				if(childPid == 0){//child
					//determine new path to search in
					char * pathCopy = calloc(sizeof(char), strlen(name) + 1);
					strcpy(pathCopy, name);
					char ** newArgv = calloc(sizeof(char*), 4);
					char depth[30];
					sprintf(depth, "%lu", initDepth-1);
					newArgv[0] = argv[0];
					newArgv[1] = pathCopy;
					newArgv[2] = argv[2];
					newArgv[3] = depth;
					execvp(argv[0], newArgv);
					return 0;
				} 
				else{
					nChildren ++;
					childrenPids = realloc(childrenPids, sizeof(pid_t)*nChildren);
					childrenPids[nChildren-1] = childPid;
				}
			}
		}
		else if(S_ISREG(nameStat.st_mode)){//a file probably
			if(isFileText(name)){
				if(isStringPresentInFile(name, argv[2])){
					printf("Found a text file (%s) at location <%s> with string [%s] from PID %d.\n", name, argv[1], argv[2], myPid);	
				}
			}//else just skip it
		}
		free(name);
	}

	for(size_t i = 0; i < nChildren; i++){
		waitpid(childrenPids[i], NULL, 0);
	}

	free(childrenPids);
	closedir(directory);
	return 0;
}