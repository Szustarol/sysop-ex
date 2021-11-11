#ifndef DYNAMIC_LOADER_H
#define DYNAMIC_LOADER_H
#include <stddef.h>
#include <dlfcn.h>
#include <stdbool.h>

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

#define DYNAMIC_IMPORT(MAKER)\
	MAKER(bt_MainTable_t *, bt_createMainTable, size_t nEntries)\
	MAKER(void, bt_removeBlock, bt_MainTable_t * table, size_t blockIndex)\
	MAKER(void, bt_removeEntry, bt_MainTable_t * table, size_t blockIndex, size_t entryIndex)\
	MAKER(void, bt_deleteMainTable, bt_MainTable_t * table)\
	MAKER(bool, bt_mergeFiles, char * outputFileName, size_t nFiles, char ** fileSequence)\
	MAKER(void, bt_print, bt_MainTable_t * table)\
	MAKER(char**, bt_mergeSequences, size_t nSequences, char *** fileNames)\
	MAKER(size_t, bt_blockFromFile, char * fileName, bt_MainTable_t * table)\
	MAKER(size_t, bt_blockSize, bt_MainTable_t * table, size_t blockIndex)\
	MAKER(size_t, bt_getFreeBlocks, bt_MainTable_t * table)\
	MAKER(size_t, bt_getBlockSize, bt_MainTable_t * table, size_t blockIndex)

#define MAKE_POINTERS(ret_type, name, ...)\
	ret_type ((*name##_dloaded)) (__VA_ARGS__);

#define MAKE_POINTERS_EXTERN(ret_type, name, ...)\
	extern ret_type ((*name##_dloaded))(__VA_ARGS__);

#define LOAD_POINTERS(ret_type, name, ...)\
	name##_dloaded = dlsym(dlHandle, #name);

DYNAMIC_IMPORT(MAKE_POINTERS_EXTERN);

#define bt_createMainTable (*bt_createMainTable_dloaded)
#define bt_removeEntry (*bt_removeEntry_dloaded)
#define bt_removeBlock (*bt_removeBlock_dloaded)
#define bt_deleteMainTable (*bt_deleteMainTable_dloaded)
#define bt_mergeFiles (*bt_mergeFiles_dloaded)
#define bt_print (*bt_print_dloaded)
#define bt_mergeSequences (*bt_mergeSequences_dloaded)
#define bt_blockFromFile (*bt_blockFromFile_dloaded)
#define bt_blockSize (*bt_blockSize_dloaded)
#define bt_getFreeBlocks (*bt_getFreeBlocks_dloaded)
#define bt_getBlockSize (*bt_getBlockSize_dloaded)


void dynamicLoad();
void dynamicClose();

#endif