#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mailOpen(char * option){
	FILE * mail = popen("mail -N", "w");
	if(!mail){
		puts("Unable to open mail program.");
		return;
	}
	char command[60];
	sprintf(command, "%s\nheaders\nq", option);
	fputs(command, mail);
	pclose(mail);
}

void mailSend(char * to, char * title, char * content){
	char * command = malloc(sizeof(char)*(strlen(title) + strlen(content) + 60));
	sprintf(command, "mail -s \"%s\" %s", title, to);
	FILE * mail = popen(command, "w");
	if(!mail){
		puts("Unable to open mail program.");
		return;
	}
	free(command);
	fputs(content, mail);
	pclose(mail);
}

int main(int argc, char ** argv){
	if(argc == 2){
		if(strcmp(argv[1], "data") == 0){
			mailOpen("sort date");
		}
		else if(strcmp(argv[1], "nadawca") == 0){
			mailOpen("sort from");
		}
		else{
			printf("Invalid argument %s\n", argv[1]);
		}
	}
	else if(argc == 4){
		mailSend(argv[1], argv[2], argv[3]);
	}
	else{
		puts("Usage:");
		puts("program data|nadawca");
		puts("program <to> <title> <content>");
	}
	return 0;
}