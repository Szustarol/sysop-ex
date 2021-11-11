#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mergerInputParser.h"

#ifdef DYNAMIC
	#include "dynamicLoader.h"
#else
	#include "BlockTable.h"
#endif

#ifdef TEST
	#include "merger-tests/mergerTester.h"
#endif

int main(int argc, char ** argv){

	#ifdef DYNAMIC
		puts("Dynamic load");
		dynamicLoad();
	#endif

	#ifdef TEST
		puts("Test mode enabled.");
		runTests();
		puts("Tests done");
	#endif


	bt_MainTable_t * table = NULL;
	char * line = NULL;
	char * backup = NULL;
	size_t bufferSize;
	puts("Type help to see available commands");
	while(true){
		printf(">");
		size_t numRead = getline(&line, &bufferSize, stdin);
		free(backup);
		backup = calloc(sizeof(char), strlen(line)+1);
		strcpy(backup, line);
		backup[numRead-1] = 0x0;
		line[numRead-1] = 0x0;
		char * token = strtok(line, " ");
		if(strcmp(token, "help") == 0){
			printHelp();
		}
		else if(strcmp(token, "create_table") == 0){
			table = parseCreateTable(table, token);
		}
		else if(strcmp(token, "print") == 0){
			parsePrint(table);
		}
		else if(strcmp(token, "remove_block") == 0){
			parseRemoveBlock(table, token);
		}
		else if(strcmp(token, "remove_entry") == 0){
			parseRemoveEntry(table, token);
		}
		else if(strcmp(token, "merge_files") == 0){
			parseMergeFiles(table, token, line, backup);
		}
		else if(strcmp(token, "exit") == 0){
			break;
		}
		else{
			puts("Invalid command. Type help for more info");
		}
	}
	if(table != NULL)
		bt_deleteMainTable(table);
	free(line);

	#ifdef DYNAMIC
		dynamicClose();
	#endif
	return 0;
}