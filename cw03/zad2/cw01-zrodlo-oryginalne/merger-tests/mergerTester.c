#include "mergerTester.h"
#ifdef DYNAMIC
	#include "../dynamicLoader.h"
#else
	#include "../BlockTable.h"
#endif
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdint.h>

static struct tms tmsStart;
static clock_t realStart;

void timeMeasureInit(void){
	realStart = times(&tmsStart);
}

void timeMeasureCheck(void){
	struct tms tmsNow;
	clock_t realEnd = times(&tmsNow);
	printf("\tWall time: %f seconds\n\tSystem time: %f seconds\n\tUser time: %f seconds\n",
		(double)(realEnd-realStart)/sysconf(_SC_CLK_TCK),
		(double)(tmsNow.tms_stime - tmsStart.tms_stime)/sysconf(_SC_CLK_TCK),
		(double)(tmsNow.tms_utime - tmsNow.tms_utime)/sysconf(_SC_CLK_TCK));
}

#define N_TEST_FILES 6

char randomizeFileIndex(void){
	int r = rand() % N_TEST_FILES + 1;
	return '0' + r;
}

void mergeSequenceLengthTest(void){
	int pairsSize [] = {30, 500, 1000};
	int nPairTypes = sizeof(pairsSize)/sizeof(int);
	char * pairsFileNames [3][2] = {
		{"merger-tests/smallFiles/small_3","merger-tests/smallFiles/small_4"},
		{"merger-tests/mediumFiles/medium_1","merger-tests/mediumFiles/medium_2"},
		{"merger-tests/largeFiles/large_1","merger-tests/largeFiles/large_2"}
	};

	char * fileSizeNames [3] = {
		"small - 10 lines 40 chars each",
		"medium - 1000 lines 80 chars each",
		"large - 30000 lines 80 chars each"
	};
	char *** sequence = NULL;

	for(int pIndex = 0; pIndex < nPairTypes; pIndex++){
		int nPairs = pairsSize[pIndex];
		
		sequence = calloc(sizeof(char**), nPairs);

		for(int i = 0; i < 3; i++){
			printf("Merging %d pairs of type: %s (no forks)\n", nPairs, fileSizeNames[i]);
			for(int j = 0; j < nPairs; j++){
				sequence[j] = malloc(sizeof(char*) * 2);
				sequence[j][0] = calloc(sizeof(char), strlen(pairsFileNames[i][0])+1);
				sequence[j][1] = calloc(sizeof(char), strlen(pairsFileNames[i][1])+1);
				strcpy(sequence[j][0], pairsFileNames[i][0]);
				strcpy(sequence[j][1], pairsFileNames[i][1]);
				sequence[j][0][strlen(sequence[j][0])-1] = randomizeFileIndex();
				sequence[j][1][strlen(sequence[j][1])-1] = randomizeFileIndex();
			}

			timeMeasureInit();
			char ** outputFileNames = bt_mergeSequences(nPairs, sequence);
			timeMeasureCheck();

			for(int j = 0; j < nPairs; j++){
				free(sequence[j][0]);
				free(sequence[j][1]);
				free(sequence[j]);
			}

			for(int result = 0; result < nPairs; result++){
				remove(outputFileNames[result]);
				free(outputFileNames[result]);
			}
			free(outputFileNames);
		}


		free(sequence);
	}
}

void blockSaveAndFreeTest(void){
	int nBlocks [3] = {10, 500, 2000};
	char * blockFileNames [3] = {
		"merger-tests/smallFiles/small_1",
		"merger-tests/mediumFiles/medium_1",
		"merger-tests/largeFiles/large_1"
	};

	char * fileSizeNames [3] = {
		"small - 10 lines 40 chars each",
		"medium - 1000 lines 80 chars each",
		"large - 30000 lines 80 chars each"
	};

	for(int i = 0; i < 3; i++){
		int blocks = nBlocks[i];
		for(int j = 0; j < 3; j++){
			bt_MainTable_t * table;
			table = bt_createMainTable(blocks);
			printf("Reading %d blocks of type: %s\n", blocks, fileSizeNames[j]);
			timeMeasureInit();
			for(int b = 0; b < blocks; b++){
				bt_blockFromFile(blockFileNames[j], table);
			}
			timeMeasureCheck();
			printf("Freeing %d blocks of type: %s\n", blocks, fileSizeNames[j]);
			timeMeasureInit();
			bt_deleteMainTable(table);
			timeMeasureCheck();
		}
	}
}

void blockRepeatedAddTest(void){
	int nCycles [3] = {10, 500, 2000};
	char * blockFileNames [3] = {
		"merger-tests/smallFiles/small_2",
		"merger-tests/mediumFiles/medium_2",
		"merger-tests/largeFiles/large_2"
	};

	char * fileSizeNames [3] = {
		"small - 10 lines 40 chars each",
		"medium - 1000 lines 80 chars each",
		"large - 30000 lines 80 chars each"
	};

	bt_MainTable_t * table;
	table = bt_createMainTable(1);

	for(int i = 0; i < 3; i++){
		int cycles = nCycles[i];
		for(int j = 0; j < 3; j++){
			printf("Adding and removing block of type: %s - %d times\n", fileSizeNames[j], cycles);
			timeMeasureInit();
			for(int i = 0; i < cycles; i++){
				bt_blockFromFile(blockFileNames[j], table);
				bt_removeBlock(table, 0);
			}
			timeMeasureCheck();
		}
	}

	bt_deleteMainTable(table);

}

void runTests(void){
	//blockRepeatedAddTest();
	//blockSaveAndFreeTest();
	mergeSequenceLengthTest();
}
