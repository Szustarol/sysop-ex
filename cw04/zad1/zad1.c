#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>

typedef enum{
	IGNORE,
	HANDLER,
	MASK,
	PENDING
}mode;

void SIGUSR_handler(int signo){
	puts("Received SIGUSR");
}

void checkPending(void){
	sigset_t pendingSet;
	sigpending(&pendingSet);
	if(sigismember(&pendingSet, SIGUSR1))
		puts("SIGUSR is pending.");
	else
		puts("No signal pending.");
}

int main(int argc, char ** argv){
	if (argc < 2){
		puts("Supply an option: ignore|handler|mask|pending");
		return 0;
	}

	mode opMode;

	if(strcmp(argv[1], "ignore") == 0){
		opMode = IGNORE;
	}
	else if(strcmp(argv[1], "handler") == 0){
		opMode = HANDLER;
	}
	else if(strcmp(argv[1], "mask") == 0){
		opMode = MASK;
	}
	else if(strcmp(argv[1], "pending") == 0){
		opMode = PENDING;
	}
	else{
		puts("Invalid option.");
		return 0;
	}
	
	bool runningAsExecChild = false;
	
	if(argc == 3 && strcmp(argv[2], "CHILD_EXEC") == 0){
		runningAsExecChild = true;
		puts("Running as executed child");
	}
	else{
		puts("Running as parent");
		switch(opMode){
			case IGNORE:
				signal(SIGUSR1, SIG_IGN);
				break;
			case HANDLER:
				signal(SIGUSR1, SIGUSR_handler);
				break;	
			case MASK:
			case PENDING:{
				sigset_t blockingSet;
				sigemptyset(&blockingSet);
				sigaddset(&blockingSet, SIGUSR1);
				sigprocmask(SIG_BLOCK, &blockingSet, NULL);
				}
				break;	
		}
	}
	
	if(!(runningAsExecChild && opMode == PENDING))
		raise(SIGUSR1);
	
	if(opMode == PENDING){
		checkPending();
	} 
	
	if(runningAsExecChild){
		puts("Child process quitting without interruption");
		return 0;
	}
	
	//testing for fork
	pid_t childPid;
	puts("Starting child with fork");
	if((childPid = fork()) == 0){

		if(opMode != PENDING){
			raise(SIGUSR1);	
		}
		else{
			checkPending();
		}
		puts("Child process quitting without interruption");
		return 0;
	}
	wait(NULL);
	puts("Forked child finished execution.");
	puts("Starting child with exec");
	if((childPid = fork()) == 0){
		char ** newArgv = malloc(sizeof(char*)*(4));
		for(size_t i = 0; i < 2; i++){
			newArgv[i] = malloc(sizeof(char)*strlen(argv[i]));
			strcpy(newArgv[i], argv[i]);
		}
		newArgv[2] = "CHILD_EXEC";
		newArgv[3] = NULL;
		//whole memory is recreated after exec, no need to free
		execv(argv[0], newArgv);
		puts("Not now");
		    printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
		return 0; //in case something goes wrong
	}
	wait(NULL);
	puts("Executed child finished execution.");
		
	return 0;
}
