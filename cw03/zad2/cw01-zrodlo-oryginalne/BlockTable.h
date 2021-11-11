#ifndef BLOCKTABLE_H
#define BLOCKTABLE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct{
	bool * valid;
	char ** content;
	size_t size;
	size_t capacity;
}bt_Block_t;

typedef struct{
	size_t size;
	bt_Block_t ** blocks;
	bool * valid;
}bt_MainTable_t;

extern size_t bt_tempFileIndex;

bt_MainTable_t * bt_createMainTable(size_t nEntries);
void bt_removeBlock(bt_MainTable_t * table, size_t blockIndex);
void bt_removeEntry(bt_MainTable_t * table, size_t blockIndex, size_t entryIndex);
void bt_deleteMainTable(bt_MainTable_t * table);
bool bt_mergeFiles(char * outputFileName, size_t nFiles, char ** fileSequence);
void bt_print(bt_MainTable_t * table);
char ** bt_mergeSequences(size_t nSequences, char *** fileNames);
size_t bt_blockFromFile(char * fileName, bt_MainTable_t * table);
size_t bt_blockSize(bt_MainTable_t * table, size_t blockIndex);
size_t bt_getFreeBlocks(bt_MainTable_t * table);
size_t bt_getBlockSize(bt_MainTable_t * table, size_t blockIndex);

#endif