#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
	if(argc < 2){
		puts("Please specify number of children");
		return 0;
	}
	size_t n;
	if((n = atoi(argv[1])) == 0){
		puts("Invalid number of children specified");
		return 0;
	}



	//to avoid zombie processes
	pid_t * childrenPids = calloc(sizeof(pid_t), n);

	puts("parent: starting child processes");
	for(size_t i = 0; i < n; i++){
		pid_t chPid = fork();
		if(chPid == 0){
			printf("child: hello, from child %lu\n", i);
			return 0;
		}
		else{
			childrenPids[i] = chPid;
		}
	}
	puts("parent: waiting for children execution to finish");
	for(size_t i = 0; i < n; i++){
		waitpid(childrenPids[i], NULL, 0);
	}
	puts("parent: done");
	free(childrenPids);
	return 0;
}