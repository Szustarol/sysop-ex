#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/times.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>

struct __tms{
	clock_t userTime;
	clock_t systemTime;
	clock_t realTime;
}__tms_default={0,0,0};

typedef struct __tms timeMeasurement;

typedef struct{
	clock_t realStart;
	struct tms tmsStart;
}measurementStart;

measurementStart timeMeasureInit(void){
	measurementStart ms;
	ms.realStart = times(&ms.tmsStart);
	return ms;
}

void updateTimeStructure(timeMeasurement *tm, measurementStart ms){
	struct tms tmsNow;
	clock_t realEnd = times(&tmsNow);

	tm->userTime += tmsNow.tms_cutime - ms.tmsStart.tms_cutime;
	tm->systemTime += tmsNow.tms_cstime - ms.tmsStart.tms_cstime;
	tm->realTime += realEnd-ms.realStart;
}

void printResult(timeMeasurement * tm, int n){
	printf("Execution time of %d executions: \n\t\
		Wall time: %f seconds (%f average)\n\
		\tSystem time: %f seconds (%f average)\n\
		\tUser time: %f seconds (%f average)\n",
		n,
		(double)(tm->realTime)/sysconf(_SC_CLK_TCK), (double)(tm->realTime)/sysconf(_SC_CLK_TCK)/n,
		(double)(tm->systemTime)/sysconf(_SC_CLK_TCK), (double)(tm->systemTime)/sysconf(_SC_CLK_TCK)/n,
		(double)(tm->userTime)/sysconf(_SC_CLK_TCK), (double)(tm->userTime)/sysconf(_SC_CLK_TCK)/n);
}

int main(int argc, char ** argv){

	if(argc < 3){
		puts("Usage: ntimes <number of tests> <program> [program parameters]");
		return 0;
	}

	if(atoi(argv[1]) == 0){
		puts("Invalid execution count.");
		return 0;
	}

	if(access(argv[2], X_OK) != 0){
		puts("Requested program can't be found");
		return 0;
	}

	int nChildren = atoi(argv[1]);
	timeMeasurement tm = __tms_default;

	for(size_t i = 0; i < nChildren; i++){
		pid_t pid = fork();
		if(pid == 0){
			int fd = open("/dev/null", O_WRONLY);
			dup2(fd, 1);
			close(fd);
			execvp(argv[2], argv+2);
			puts("Unable to execute target file");
			return 0;
		}
		else{
			measurementStart ms = timeMeasureInit();
			wait(NULL);
			updateTimeStructure(&tm, ms);
		}
	}

	printResult(&tm, nChildren);

	return 0;
}