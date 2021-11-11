#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

typedef enum{
	KILL,
	SIGQUEUE,
	SIGRT
}SendingMode;

void blockSIGUSRs(){
	sigset_t blockSet;
	sigaddset(&blockSet, SIGUSR2);
	sigaddset(&blockSet, SIGUSR1);
	sigaddset(&blockSet, SIGRTMIN);
	sigaddset(&blockSet, SIGRTMIN+1);
	sigprocmask(SIG_BLOCK, &blockSet, NULL);
}

void nullHandler(int signo){}

void setSignals(void (* f)(int)){
	signal(SIGUSR1, f);
	signal(SIGUSR2, f);
	signal(SIGRTMIN, f);
	signal(SIGRTMIN+1, f);
}

void setSignalsQueued(void (*f)(int, siginfo_t *, void*)){
	struct sigaction sa;
	sa.sa_sigaction = f;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sigaddset(&sa.sa_mask, SIGRTMIN);
	sigaddset(&sa.sa_mask, SIGRTMIN+1);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGRTMIN, &sa, NULL);
	sigaction(SIGRTMIN+1, &sa, NULL);
}


void waitForSIGUSR(int usr1, int usr2){
	sigset_t usr2Set;
	sigfillset(&usr2Set);
	if(usr2){
		sigdelset(&usr2Set, SIGUSR2);
		sigdelset(&usr2Set, SIGRTMIN+1);
	}
	if(usr1){
		sigdelset(&usr2Set, SIGUSR1);
		sigdelset(&usr2Set, SIGRTMIN);	
	}
	sigdelset(&usr2Set, SIGINT);
	sigsuspend(&usr2Set);
}

void modeSend(int pid, int signal, SendingMode mode, int data/*only for sigqueue*/){
	switch(mode){
		case KILL:
			kill(pid, signal);
		break;
		case SIGQUEUE:
			sigqueue(pid, signal, (const union sigval){.sival_int=data});
		break;
		case SIGRT:
			if(signal == SIGUSR1)
				kill(pid, SIGRTMIN);
			else if(signal == SIGUSR2)
				kill(pid, SIGRTMIN+1);
			else
				puts("Signal not supported for this mode.");
		break;
	}
}
