#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct{
	char * program;
	char ** args; //with program name as first arg
	size_t argc;
} execData;

typedef struct{
	execData * executions;
	size_t nCommands;
	char * sequenceName;
} sequenceData;

typedef struct{
	size_t nCommands;
	char ** commands;
} commandData;

typedef struct{
	commandData * commandList;
	size_t nCommands;
	size_t nSequences;
	sequenceData * sequences;
} appData;

execData * commandsToExecutions(char ** commands, size_t nCommands){ // frees commands
	execData * result = malloc(sizeof(execData) * nCommands);
	for(size_t i = 0; i < nCommands; i++){
		char * ptr = strtok(commands[i], " ");
		char ** args = NULL;
		size_t argc = 0;
		bool firstExec = true;
		while(ptr != NULL){
			if(firstExec){
				result[i].program = strdup(ptr);
				firstExec = false;
			}
			args = realloc(args, sizeof(char*)*++argc);
			args[argc-1] = strdup(ptr);
			ptr = strtok(NULL, " ");
		}
		args = realloc(args, sizeof(char*)*(argc+1));
		args[argc] = NULL;
		result[i].args = args;
		result[i].argc = argc;
		free(commands[i]);
	}
	free(commands);
	return result;
}

char * nameStrip(char * input){ 
	char * output = malloc(sizeof(char*)*strlen(input+1));
	size_t offset = 0;
	for(size_t i = 0; i < strlen(input); i++){
		if(isdigit(input[i]) || isalpha(input[i])){
			output[offset++] = input[i];
		}
	}
	output[offset] = 0x0;
	return output;
	
}

char ** pipeSplit(char * text, size_t * rL, bool strip){ // does not free input
	char * ptr = strtok(text, "|");
	char ** result = NULL;
	size_t nS = 0;
	while(ptr != NULL){
		result = realloc(result, sizeof(char*)*(nS+1));
		if(strip){
			char * interm = strdup(ptr);
			result[nS++] = nameStrip(interm);
			free(interm);
		}
		else
			result[nS++] = strdup(ptr);
		ptr = strtok(NULL, "|");
	}
	*rL = nS;
	return result;
}


size_t readLines(char *** outputValidLines, int ** outputEqPositions, size_t lineCount, char * fileContent, size_t fileSize){
	char ** validLines = NULL;
	int * eqPositions = NULL;
	
	size_t NvalidLines = 0;
	
	size_t offset = 0;
	for(size_t lineIndex = 0; lineIndex < lineCount; lineIndex++){
		bool foundNonWhite = false;
		int eqPosition = -1;
		for(size_t i = offset; i < fileSize; i++){
			if(isalpha(fileContent[i]) || isdigit(fileContent[i]))
				foundNonWhite = true;
			if(fileContent[i] == '='){
				foundNonWhite = true;
				eqPosition = i-offset;
			}
			if(fileContent[i] == '#' || fileContent[i] == '\n' || fileContent[i] == 0x0){
				if(foundNonWhite){
					validLines = realloc(validLines, sizeof(char*)*++NvalidLines);
					validLines[NvalidLines-1] = strndup(fileContent + offset, i-offset);
					eqPositions = realloc(eqPositions, sizeof(int)*NvalidLines);
					eqPositions[NvalidLines-1] = eqPosition;
				}
				//else skip this line
				while(fileContent[i] != '\n' && fileContent[i] != 0x0)
					i++;
				offset = i+1;
				break;
			}
		}
	}
	
	*outputValidLines = validLines;
	*outputEqPositions = eqPositions;
	
	return NvalidLines;
}


appData fileParse(char * fileContent, size_t fileSize){
	
	appData result = {.commandList = NULL, .sequences = NULL, .nCommands = 0};

	//find all lines, skip newlines and skip coments (starting with #)
	size_t lineCount = 0;

	for(size_t i = 0; i < fileSize; i++){
		if(fileContent[i] == '\n')
			lineCount += 1;
	}
	char ** validLines = NULL;
	int * eqPositions = NULL;

	size_t NvalidLines = readLines(&validLines, &eqPositions, lineCount, fileContent, fileSize);

	for(size_t i = 0; i < NvalidLines; i++){
		if(eqPositions[i] == -1){ //a command, not a variable definition
			size_t nNames;
			char ** varNames = pipeSplit(validLines[i], &nNames, true);
			result.commandList = realloc(result.commandList, sizeof(commandData)*++result.nCommands);
			result.commandList[result.nCommands-1].commands = varNames;
			result.commandList[result.nCommands-1].nCommands = nNames;
		}
		else{
			//variable definition
			validLines[i][eqPositions[i]] = 0x0; 
			char * name = nameStrip(validLines[i]);
			size_t nCommands;
			char ** commands = pipeSplit(validLines[i] + eqPositions[i] + 1, &nCommands, false);
			execData * executions = commandsToExecutions(commands, nCommands);
			sequenceData dat = {.nCommands = nCommands, .sequenceName = name, .executions = executions};
			result.sequences = realloc(result.sequences, sizeof(sequenceData)*++result.nSequences);
			result.sequences[result.nSequences-1] = dat;
		}
		free(validLines[i]);
	}
	
	free(validLines);
	free(eqPositions);
	
	return result;
}


void freeAppData(appData * data){
	for(size_t i = 0; i < data->nSequences; i++){
		free(data->sequences[i].sequenceName);
		for(size_t j = 0; j < data->sequences[i].nCommands; j++){
			for(size_t k = 0; k < data->sequences[i].executions[j].argc; k++){
				free(data->sequences[i].executions[j].args[k]);
			}
			free(data->sequences[i].executions[j].program);
			free(data->sequences[i].executions[j].args);
		}
		free(data->sequences[i].executions);
	}
	
	free(data->sequences);
	
	for(size_t i = 0; i < data->nCommands; i++){
		for(size_t j = 0; j < data->commandList[i].nCommands; j++){
			free(data->commandList[i].commands[j]);
		}
		free(data->commandList[i].commands);
	}
	free(data->commandList);
}


void prettyPrintAppData(appData * data){
	puts("\nVariables");
	for(size_t i = 0; i < data->nSequences; i++){
		printf("\tVariable name: '%s'\n", data->sequences[i].sequenceName);
		for(size_t j = 0; j < data->sequences[i].nCommands; j++){
			printf("\t\tProgram name: '%s'\n", data->sequences[i].executions[j].program);
			for(size_t k = 0; k < data->sequences[i].executions[j].argc; k++){
				printf("\t\t\tProgram argument: '%s'\n", data->sequences[i].executions[j].args[k]);
			}
		}
	}
	
	puts("\nCommands");
	for(size_t i = 0; i < data->nCommands; i++){
		puts("\tCommand:");
		for(size_t j = 0; j < data->commandList[i].nCommands; j++){
			printf("\t\tCommand entry: '%s'\n", data->commandList[i].commands[j]);
		}
	}
}

sequenceData * commandSearch(appData * data, char * cName){
    for(size_t i = 0; i < data->nSequences; i++){
        if(strcmp(data->sequences[i].sequenceName, cName) == 0){
            return data->sequences + i;
        }
    }
    return NULL;
}

void createPipelines(appData  * data, char **** pP, char ***** pA, size_t ** nP){
    size_t nPipeLines = data->nCommands;

    char *** pipelinePrograms = calloc(nPipeLines, sizeof(char**));
    char **** pipelineArgs = calloc(nPipeLines, sizeof(char***));
    size_t * nPrograms = calloc(nPipeLines, sizeof(size_t));
    for(int i = 0; i < nPipeLines; i++){
        //i-pipeline index
        for(int j = 0; j < data->commandList[i].nCommands; j++){
            //j - command index
            //find command with given name
            char * cName = data->commandList[i].commands[j];
            sequenceData * seq = commandSearch(data, cName);
            if(seq == NULL){
                printf("Variable name not found: %s\n", cName);
                return;
            }
            //add n programs to list of programs in this pipeline
            nPrograms[i] += seq->nCommands;
            pipelinePrograms[i] = realloc(pipelinePrograms[i], sizeof(char**)*nPrograms[i]);
            pipelineArgs[i] = realloc(pipelineArgs[i], sizeof (char***)*nPrograms[i]);
            for(int k = 0; k < seq->nCommands; k++){
                pipelinePrograms[i][nPrograms[i]-seq->nCommands+k] = seq->executions[k].program;
                pipelineArgs[i][nPrograms[i]-seq->nCommands+k] = seq->executions[k].args;
            }

        }
    }

    *pP = pipelinePrograms;
    *pA = pipelineArgs;
    *nP = nPrograms;
}

void freePipelines(char *** pipelinePrograms, char **** pipelineArgs, size_t * nPrograms, size_t nPipeLines){
    for(size_t i = 0; i < nPipeLines; i++){
        free(pipelinePrograms[i]);
        free(pipelineArgs[i]);
    }
    free(pipelinePrograms);
    free(pipelineArgs);
    free(nPrograms);
}


void executePipelines(appData * data, char *** pipelinePrograms, char **** pipelineArgs, size_t * nPrograms){
    size_t nPipeLines = data->nCommands;

    for(size_t i = 0; i < nPipeLines; i++){
        if(nPrograms[i] == 0)
            continue;
        pid_t * children = calloc(nPrograms[i], sizeof(pid_t));
        size_t nChildren = nPrograms[i];
        printf("\n\n\nStarting pipeline %lu\n", i);
        printf("\t");
        for(size_t j = 0; j < nPrograms[i]; j++){
            printf("%s  [", pipelinePrograms[i][j]);
            for(size_t k = 0; ; k++){
                if(pipelineArgs[i][j][k] == NULL)
                    break;
                printf("%s ", pipelineArgs[i][j][k]);
            }
            printf("]");
            if(j != nPrograms[i] -1)
                printf(" | ");
        }
        printf("\n");
        int * fds = malloc(sizeof(int) * (nPrograms[i]-1) * 2);
        int outputFds[2];
        pipe(outputFds);

        for(size_t j = 0; j < nPrograms[i]-1; j++){
            pipe(fds+j*2);
        }

        int childProgIndex = -1;
        for(size_t j = 0; j < nPrograms[i]; j++){
            pid_t p;
            if((p = fork()) == 0){
                childProgIndex = j;
                int taken1 = -1;
                int taken2 = -1;
                if(j == nPrograms[i] -1){//output to parent
                    dup2(outputFds[1], STDOUT_FILENO);
                }
                if(j != 0){
                    taken1 = 2*j-2;
                    dup2(fds[taken1], STDIN_FILENO);
                }
                if(j != nPrograms[i] -1){
                    taken2 = 2*j+1;
                    dup2(fds[taken2], STDOUT_FILENO);
                    close(outputFds[1]);
                }
                close(outputFds[0]);

                //close remaining fds
                for(size_t g = 0; g < 2*(nPrograms[i]-1); g++){
                    if(g != taken1 && g != taken2){
                        close(fds[g]);
                    }
                }
                // do not exec yet - first start all processes
                break;
            }
            else{
                children[j] = p;
            }
        }
        if(childProgIndex == -1)
            for(size_t j = 0; j < nPrograms[i]-1; j++) {
                close(fds[j * 2]);
                close(fds[j * 2 + 1]);
            }

        if(childProgIndex != -1){
            execvp(pipelinePrograms[i][childProgIndex], pipelineArgs[i][childProgIndex]);
            exit(0);
        }
        int flags = fcntl(outputFds[0], F_GETFL, 0);
        fcntl(outputFds[0], F_SETFL, flags | O_NONBLOCK);

        close(outputFds[1]);

        for(size_t j = 0; j < nChildren; j++){
            if(j < nChildren-1)
                waitpid(children[j], NULL, 0);
            else{
                puts("Pipeline output:");
                char buffer[60];
                int nRead;
                do{
                    while ((nRead = read(outputFds[0], buffer, 60)) > 0){
                        printf("%.*s", nRead, buffer);
                    }
                }while(waitpid(children[j], NULL, WNOHANG) == 0);
                puts("");
            }
        }

        close(outputFds[0]);
        free(fds);
        free(children);
    }
}

void startNRun(appData  * data){
    //glue all commands together
    size_t nPipeLines = data->nCommands;
    char *** pipelinePrograms;
    char **** pipelineArgs;
    size_t * nPrograms;
    createPipelines(data, &pipelinePrograms, &pipelineArgs, &nPrograms);

    //start all the processes
    executePipelines(data, pipelinePrograms, pipelineArgs, nPrograms);

    freePipelines(pipelinePrograms, pipelineArgs, nPrograms, nPipeLines);
}

int main(int argc, char ** argv){
	if(argc < 2){
		puts("Pass the input file as a parameter.");
		return 0;
	}

	FILE * input = fopen(argv[1], "r");
	if(!input){
		puts("Unable to open requested file.");
		return 0;
	}

	fseek(input, 0, SEEK_END);
    size_t fileSize = ftell(input);
	fseek(input, 0, SEEK_SET);
	char * fileContent = malloc(sizeof(char)*(fileSize+1));
	fileContent[fileSize] = 0;
	fread(fileContent, sizeof(char), fileSize, input);
	fclose(input);

	appData dat = fileParse(fileContent, fileSize);
	prettyPrintAppData(&dat);
    	startNRun(&dat);
	freeAppData(&dat);

	free(fileContent);

	return 0;
}
