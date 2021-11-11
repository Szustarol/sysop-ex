#include "mergerInputParser.h"

void printHelp(void){
	puts("Available commands:");
	puts("\tcreate_table [n] - create table of size n and remove previous \n\t\ttables if exist");
	puts("\tmerge_files [filename1]:[filename2] [filename3]:[filename4] ... - \n\t\tmerge files separated by :");
	puts("\tremove_block [index] - remove block with index");
	puts("\tremove_row [block_index] [row_index] - remove row from block given their indexes");
	puts("\tprint - prints out table content");
	puts("\texit - close the software");
}

bool isValidInteger(char * string){
	while(*string != 0x0){
		if(!isdigit(*(string++))){
			return false;
		}
	}
	return true;
}

char ** extractFileNames(char * input){
	puts(input);
	//search for : first
	size_t sepPosition = 0;
	bool found = false;
	char * t = input;
	while(*t != 0x0 && *t != ' '){
		if(*t == ':'){
			found = true;
			break;
		}
		sepPosition++;
		t++;
	}

	if(found == false)
		return NULL;

	if(input[sepPosition+1] == 0 || input[sepPosition+1] == ' ')
		return NULL;
 
	if(input[sepPosition-1] == ' ')
		return NULL;

	size_t secondLen = 0;
	t = input + sepPosition + 1;
	while(*t != 0 && *t != ' '){
		t++;
		secondLen++;
	}


	char * firstFileName = calloc(sizeof(char), sepPosition+1);
	char * secondFileName = calloc(sizeof(char), secondLen+1);

	strncpy(firstFileName, input, sepPosition);
	strncpy(secondFileName, input+sepPosition+1, secondLen);

	char ** output = calloc(sizeof(char*), 2);
	output[0] = firstFileName; output[1] = secondFileName;
	return output;
}

bt_MainTable_t * parseCreateTable(bt_MainTable_t * table, char * token){
	token = strtok(NULL, " ");
	if(token == NULL || !isValidInteger(token)){
		puts("Invalid parameter. Parameter must be an integer.");
	}
	else{
		if(table != NULL){
			bt_deleteMainTable(table);
		}
		table = bt_createMainTable(atoi(token));
	}
	return table;
}

void parsePrint(bt_MainTable_t * table){	
	if(table == NULL)
		puts("No table allocated.");
	else{
		bt_print(table);
	}
}

void parseRemoveBlock(bt_MainTable_t * table, char * token){
	token = strtok(NULL, " ");
	if(token == NULL || !isValidInteger(token)){
		puts("Invalid parameter. Parameter must be an integer.");
	}
	else{
		size_t index = atoi(token);
		if(table == NULL || index > table->size){
			puts("No table is allocated or requested index does not fit in table");
		}
		else{
			if(table->valid[index]){
				bt_removeBlock(table, index);
			}
			else{
				puts("Requested block is not allocated");
			}
		}
	}
}

void parseRemoveEntry(bt_MainTable_t * table, char * token){
	int blockNumber, entryNumber;
	token = strtok(NULL, " ");
	if(token == NULL || !isValidInteger(token)){
		puts("Invalid parameter. PArameter must be an integer.");
	}
	else{
		blockNumber = atoi(token);
		token = strtok(NULL, " ");
		if(token == NULL || !isValidInteger(token)){
			puts("Invalid parameter. Parameter must be an integer.");
		}
		else{
			entryNumber = atoi(token);
			if(table->valid[blockNumber]){
				if(table->blocks[blockNumber]->valid[entryNumber]){
					bt_removeEntry(table, blockNumber, entryNumber);
				}
				else{
					puts("Requested entry is not allocated.");
				}
			}
			else{
				puts("Requested block is not allocated");
			}
		}
	}	
}

void parseMergeFiles(bt_MainTable_t * table, char * token, char * line, char * backup){
	char *** sourceFiles = NULL;
	// find number of remaining pairs
	size_t nInputs = 0;
	while((token = strtok(NULL, " ")) != NULL){
		nInputs++;
	}
	if(table == NULL || bt_getFreeBlocks(table) < nInputs){
		printf("Current table has not enough blocks to fit the requested ammount of pairs\n");
	}
	else{
		sourceFiles = calloc(sizeof(char**), nInputs);
		strcpy(line, backup);
		token = strtok(line, " ");
		size_t valid = 0;
		while((token = strtok(NULL, " ")) != NULL){
			char ** output = extractFileNames(token);
			if(output == NULL){
				printf("Invalid pair: %s\n", token);
			}
			else{
				sourceFiles[valid++] = output; 
			}
		}
		char ** tempFileNames = bt_mergeSequences(valid, sourceFiles);
		if(tempFileNames == NULL){
			puts("Invalid files specified.");
		}
		else{
			for(int i = 0; i < valid; i++){
				free(sourceFiles[i][0]);
				free(sourceFiles[i][1]);
				free(sourceFiles[i]);
				bt_blockFromFile(tempFileNames[i], table);
				remove(tempFileNames[i]);
				free(tempFileNames[i]);
			}
			free(tempFileNames);
			free(sourceFiles);	
		}
	}
}