#include "BlockTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

size_t bt_tempFileIndex = 0;

bt_MainTable_t * bt_createMainTable(size_t nEntries){
	bt_MainTable_t * newTable = calloc(sizeof(bt_MainTable_t), 1);
	newTable->size = nEntries;
	newTable->blocks = calloc(sizeof(bt_Block_t*), nEntries);
	newTable->valid = calloc(sizeof(bool), nEntries);
	return newTable;
}


void __removeBlock(bt_MainTable_t * table, size_t blockIndex){
	bt_Block_t * block = table->blocks[blockIndex];
	for(size_t e = 0; e < block->capacity; e++){
		if(block->valid[e])
			free(block->content[e]);
	}
	free(block->content);
	free(block->valid);
	free(block);
}

void bt_deleteMainTable(bt_MainTable_t * table){
	for(size_t i = 0; i < table->size; i++){
		if (table->valid[i]){
			__removeBlock(table, i);
		}
	}
	free(table->blocks);
	free(table->valid);
	free(table);
}

bool bt_mergeFiles(char * outputFileName, size_t nFiles, char ** fileSequence){
	//open each of the files
	FILE ** files = calloc(sizeof(FILE*), nFiles);
	long * fileSizes = calloc(sizeof(long), nFiles);
	long allSize = 0;
	long biggestFile = 0;
	for(size_t i = 0; i < nFiles; i++){
		files[i] = fopen(fileSequence[i], "r");
		if(files[i] == NULL){
			free(fileSizes);
			for(size_t i = 0; i < nFiles; i++){
				if(files[i] != NULL)
					fclose(files[i]);
			}
			free(files);
			return false;
		}
		//get size of each file
		fseek(files[i], 0L, SEEK_END);
		fileSizes[i] = ftell(files[i]);
		fseek(files[i], 0L, SEEK_SET);
		allSize += fileSizes[i];
		biggestFile = fileSizes[i] > biggestFile ? fileSizes[i] : biggestFile;
	}

	char * buffer = calloc(sizeof(char), biggestFile);

	FILE * output = fopen(outputFileName, "w");

	while(allSize > 0){
		for(size_t i = 0; i < nFiles; i++){
			if(fileSizes[i] < 1)
				continue;
			fgets(buffer, allSize, files[i]);
			long inputSize = strlen(buffer);
			fileSizes[i] -= inputSize;
			allSize -= inputSize;
			if(buffer[0] == 0){
				fileSizes[i] -= 1;
				allSize -= 1;
			}
			else
				fwrite(buffer, sizeof(char), inputSize, output);
		}
	}	

	fseek(output, -1, SEEK_END);
	if(fgetc(output) != '\n'){
		fwrite("\n", sizeof(char), 1, output);
	}


	fclose(output);

	free(buffer);


	//close files
	for(size_t i = 0; i < nFiles; i++){
		fclose(files[i]);
	}

	free(files);
	free(fileSizes);

	return true;
}


char ** bt_mergeSequences(size_t nSequences, char *** fileNames){
	char ** outputTempFiles = calloc(sizeof(char*), nSequences);
	char * outputFileName = calloc(sizeof(char), 30);
	for(size_t i = 0; i < nSequences; i++){
		sprintf(outputFileName, "tempfile%ld", bt_tempFileIndex++);
		outputTempFiles[i] = calloc(sizeof(char), strlen(outputFileName)+1);
		strcpy(outputTempFiles[i], outputFileName);
		bool mergeResult = bt_mergeFiles(outputFileName, 2, fileNames[i]);
		if(!mergeResult){
			for(size_t j = 0; j <= i; j++){
				free(outputTempFiles[j]);
			}
			free(outputTempFiles);
			free(outputFileName);
			return NULL;
		}
	}
	free(outputFileName);

	return outputTempFiles;
}

size_t bt_blockFromFile(char * fileName, bt_MainTable_t * table){
	//find empty index in table
	size_t index = 0;
	for(; index < table->size; index++){
		if(!table->valid[index])
			break;
	}

	FILE * file = fopen(fileName, "r");
	//determine number of lines in file

	size_t lines = 1;
	size_t maxLineSize = 1;
	size_t lineSize = 1;
	while(!feof(file)){
		if(fgetc(file) == '\n'){
			lines++;
			if(++lineSize > maxLineSize)
				maxLineSize = lineSize;
			lineSize = 0;
		}
		else{
			lineSize++;
		}
	}

	fseek(file, 0L, SEEK_SET);


	table->valid[index] = true;
	bt_Block_t * block;

	block = calloc(sizeof(bt_Block_t), 1);
	table->blocks[index] = block;
	block->content = calloc(sizeof(char*), lines);
	block->valid = calloc(sizeof(bool), lines);
	for(size_t i = 0; i < lines; i++)
		block->valid[i] = true;

	char * buffer = calloc(sizeof(char), maxLineSize+1);

	for(size_t i = 0; i < lines; i++){
		fgets(buffer, maxLineSize, file);
		block->content[i] = calloc(sizeof(char), strlen(buffer)+1);
		strcpy(block->content[i], buffer);
	}

	block->size = lines;
	block->capacity = lines;

	fclose(file);

	return index;
}

size_t bt_blockSize(bt_MainTable_t * table, size_t blockIndex){
	return table->blocks[blockIndex]->size;
}

void bt_removeEntry(bt_MainTable_t * table, size_t blockIndex, size_t entryIndex){
	table->blocks[blockIndex]->valid[entryIndex] = false;
	free(table->blocks[blockIndex]->content[entryIndex]);
	table->blocks[blockIndex]->size--;
}

void bt_removeBlock(bt_MainTable_t * table, size_t blockIndex){
	__removeBlock(table, blockIndex);
	table->valid[blockIndex] = false;
}

void __printBlock(bt_Block_t * block){
	for(size_t i = 0; i < block->capacity; i++){
		if(block->valid[i]){
			printf("\t%s", block->content[i]);
		}
	}
}

void bt_print(bt_MainTable_t * table){
	printf("Content of block table: \n");
	for(size_t i = 0; i < table->size; i++){
		if(table->valid[i]){
			printf("Conent of block %lu:\n", i);
			__printBlock(table->blocks[i]);
		}
	}
}

size_t bt_getFreeBlocks(bt_MainTable_t * table){
	size_t ctr = 0;
	for(size_t i = 0; i < table->size; i++){
		if(!table->valid[i])
			ctr++;
	}
	return ctr;
}

size_t bt_getBlockSize(bt_MainTable_t * table, size_t blockIndex){
	if(!table->valid[blockIndex])
		return -1;
	return table->blocks[blockIndex]->size;
}