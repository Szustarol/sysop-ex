#include "commonDefinitions.h"


int responseCount;
int sig2Received;
int recvData;


void responseHandler(int signo){
	if(signo == SIGUSR1 || signo == SIGRTMIN)
		responseCount++;
	if(signo == SIGUSR2 || signo == SIGRTMIN+1)
		sig2Received = 1;
}

void queueResponseHandler(int signo, siginfo_t * info, void * ucontext){
	if(signo == SIGUSR1 || signo == SIGRTMIN)
		recvData = info->si_value.sival_int;
	responseHandler(signo);
}

void senderCore(pid_t catcherPid, int nSignals, SendingMode mode){
	setSignals(nullHandler);
	//tell the catcher what type of communication will be used
	sigqueue(catcherPid, SIGUSR2, (const union sigval){.sival_int=mode});
	//await ACK from catcher
	waitForSIGUSR(0, 1);

	puts("Catcher acknowledged mode notification.");

	//send all the signals
	for(size_t i = 0; i < nSignals; i++){
		modeSend(catcherPid, SIGUSR1, mode, 0);
	}

	puts("Transmission finished.");

	//end of transmission
	modeSend(catcherPid, SIGUSR2, mode, 0);


	responseCount = 0;
	sig2Received = 0;
	//wait for the response

	if(mode == SIGQUEUE)
		setSignalsQueued(queueResponseHandler);
	else
		setSignals(responseHandler);
	while(!sig2Received)
		waitForSIGUSR(1, 1);

	printf("Received %d signals.\n", responseCount);
	if(mode == SIGQUEUE)
		printf("Catcher seems to have received %d signals\n", recvData);
	printf("Should have received %d signals.\n", nSignals);
}

int main(int argc, char ** argv){
	if(argc < 4){
		puts("Usage: program [Catcher PID] [Number of signals] [Sending mode].");
		return 0;
	}

	pid_t catcherPid;
	int nSignals;

	if((catcherPid = atoi(argv[1])) < 1){
		puts("Invalid catcher PID.");
		return 0;
	}

	if((nSignals = atoi(argv[2])) < 1){
		puts("Invalid number of signals.");
		return 0;
	}

	SendingMode mode;

	if(strcmp(argv[3], "KILL") == 0){
		mode = KILL;
	}
	else if(strcmp(argv[3], "SIGQUEUE") == 0){
		mode = SIGQUEUE;
	}
	else if(strcmp(argv[3], "SIGRT") == 0){
		mode = SIGRT;
	}
	else{
		puts("Invalid mode specifier.");
		return 0;
	}

	senderCore(catcherPid, nSignals, mode);

	return 0;
}