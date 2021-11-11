#ifndef MERGER_INPUT_PARSER_H
#define MERGER_INPUT_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef DYNAMIC
	#include "dynamicLoader.h"
#else
	#include "BlockTable.h"
#endif

void printHelp(void);
bool isValidInteger(char * string);
char ** extractFileNames(char * input);
bt_MainTable_t * parseCreateTable(bt_MainTable_t * table, char * token);
void parsePrint(bt_MainTable_t * table);
void parseRemoveBlock(bt_MainTable_t * table, char * token);
void parseRemoveEntry(bt_MainTable_t * table, char * token);
void parseMergeFiles(bt_MainTable_t * table, char * token, char * line, char * backup);



#endif