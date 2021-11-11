#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

//code for testing SA_SIGINFO flag
void siginfoHandler(int signum, siginfo_t * sigInfo, void * context){
	puts("Entering SIGINFO handler");
	printf("Sending process ID: %d\n", sigInfo->si_pid);
	printf("Signal ID: %d\n", signum);
	printf("Sending user ID: %d\n", sigInfo->si_uid);
	printf("System time consumed: %lu\n", sigInfo->si_stime);
	printf("Signal file descriptor ID: %d\n", sigInfo->si_fd);
	printf("Number of attempted syscalls: %d\n", sigInfo->si_syscall);
	puts("Exiting SIGINFO handler");
}

//code for testing SA_NODEFER FLAG
volatile sig_atomic_t callCounter;

void nodeferHandler(int signo){
	const char entry_message[] = "Entering the NODEFER flag handler\n";
	write(STDOUT_FILENO, entry_message, strlen(entry_message));
	if(callCounter < 10){
		char gt_message[140];
		sprintf(gt_message, "Raising another signal from withing handler, since the counter is %d, which is less than 10\n", callCounter);
		callCounter++;
		write(STDOUT_FILENO, gt_message, strlen(gt_message));
		raise(SIGUSR1);
		while(callCounter<10);
	}
	const char end_message[] = "Handler finishes execution\n";
	write(STDOUT_FILENO, end_message, strlen(end_message));
}

//code for testing SA_RESETHAND FLAG

void resethandHandler(int signo){
	const char handler_message[] = "Entering the SA_RESETHAND handler\n";
	write(STDOUT_FILENO, handler_message, strlen(handler_message));
}


int main(int argc, char ** argv){
	puts("Testing SA_SIGINFO flag.");
	struct sigaction sig;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = SA_SIGINFO;
	sig.sa_sigaction = siginfoHandler;
	sigaction(SIGUSR1, &sig, NULL);
	raise(SIGUSR1);

	puts("\n\nTesting SA_NODEFER flag.");
	struct sigaction sig2;
	sigemptyset(&sig2.sa_mask);
	sig2.sa_flags = SA_NODEFER;
	sig2.sa_handler = nodeferHandler;
	callCounter = 0;
	sigaction(SIGUSR1, &sig2, NULL);
	raise(SIGUSR1);

	puts("\n\nTesting SA_RESETHAND flag.");
	struct sigaction sig3;
	sigemptyset(&sig3.sa_mask);
	sig3.sa_flags = SA_RESETHAND;
	sig3.sa_handler = resethandHandler;
	sigaction(SIGUSR1, &sig3, NULL);
	raise(SIGUSR1);
	puts("Handler finished, now the next raise should do the default action which is to terminate");
	puts("the process. Text \"after raise\" should not be printed");
	raise(SIGUSR1);
	puts("after raise");
	return 0;
}
