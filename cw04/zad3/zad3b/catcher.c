#include "commonDefinitions.h"
#include <unistd.h>

SendingMode receivedMode;
pid_t senderPid;
void modeReceiver(int signo, siginfo_t * info, void * ucontext){
	receivedMode = info->si_value.sival_int;
	senderPid = info->si_pid;
}

size_t nReceived;
int receivedUSR2;

void recvHandler(int signo){
	if(signo == SIGUSR1 || signo == SIGRTMIN)
		nReceived++;
	if(signo == SIGUSR2 || signo == SIGRTMIN+1)
		receivedUSR2 = 1;
}

void catcherCore(pid_t senderPid, SendingMode mode){
	//block signals by default to prevent deadlock
	blockSIGUSRs();

	//receive SIGUSR1's
	receivedUSR2 = 0;
	setSignals(recvHandler);
	while(!receivedUSR2){
		waitForSIGUSR(1, 1);
		if(!receivedUSR2)//send confirmation to the sender
			modeSend(senderPid, SIGUSR1, mode, 0);	
	}
	printf("Received %lu SIGUSR1's\n", nReceived);
	//send them back
	for(size_t i = 0; i < nReceived; i++){
		modeSend(senderPid, SIGUSR1, mode, i+1);
	}
	//EOT
	modeSend(senderPid, SIGUSR2, mode, 0);
	puts("Sending back finished.");
}

int main(int argc, char ** argv){
	pid_t myPid = getpid();
	printf("My pid is %d\n", myPid);

	// get communication mode from the sender
	struct sigaction recvAct;
	recvAct.sa_sigaction = modeReceiver;
	sigemptyset(&recvAct.sa_mask);
	sigaddset(&recvAct.sa_mask, SIGUSR2);
	recvAct.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR2, &recvAct, NULL);
	waitForSIGUSR(0, 1);

	//sigsuspend(&recvAct.sa_mask);

	if(receivedMode > SIGRT){
		puts("Received invalid mode from sender. Closing.");
		return 0;
	}
	else{
		switch(receivedMode){
			case KILL:
				puts("Sender requested KILL communication mode.");
			break;
			case SIGQUEUE:
				puts("Sender requested SIGQUEUE communication mode.");
			break;
			case SIGRT:
				puts("Sender requested SIGRT communication mode.");
			break;
		}
	}
	modeSend(senderPid, SIGUSR2, receivedMode, 0);
	catcherCore(senderPid, receivedMode);
	return 0;
}
