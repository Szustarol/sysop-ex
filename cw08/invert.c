#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

typedef enum{
	NUMBERS,
	BLOCKS
}PartitionType;

typedef struct{
	PartitionType jobType;
	int myIndex;
	int nJobs;
	int width;
	int height;
	uint ** inData;
	uint ** outData;
	long timeTaken;
}jobInfo;

void * thread_job(void * data){
	struct timespec beginT, endT;
	clock_gettime(CLOCK_REALTIME, &beginT);

	jobInfo * job = data;

	if(job->jobType == NUMBERS){
		int numStart = job->myIndex/job->nJobs*255;
		int numEnd;
		if(job->myIndex == job->nJobs-1){
			numEnd = 255;
		}
		else{
			numEnd = (job->myIndex+1)/job->nJobs*255-1;
		}

		for(int i = 0; i < job->height; i++){
			for(int j = 0; j < job->width; j++){
				int inValue = job->inData[i][j];
				if(inValue >= numStart && inValue <= numEnd){
					job->outData[i][j] = 255-inValue;
				}
			}
		}
	}
	else if(job->jobType == BLOCKS){
		int startCol = job->myIndex/job->nJobs*job->width;
		int endCol;
		if(job->myIndex == job->nJobs-1){
			endCol = job->width;
		}
		else{
			endCol = (job->myIndex+1)/job->nJobs*job->width;
		}		

		for(int i = 0; i < job->height; i++){
			for(int j = startCol; j < endCol; j++){
				job->outData[i][j] = 255-job->inData[i][j];
			}
		}

	}

	clock_gettime(CLOCK_REALTIME, &endT);

	job->timeTaken = endT.tv_sec*1e6+endT.tv_nsec/1000 - beginT.tv_sec*1e6-beginT.tv_nsec/1000;

	pthread_exit(&(job->timeTaken));
}

uint ** loadImage(FILE * input, int * width, int * height){
	char * header = malloc(sizeof(char) * 512);
	fread(header, sizeof(char), 2, input);
	header[2] = 0;
	if(strcmp(header, "P2") != 0){
		puts(header);
		return NULL;
	}
	if(fscanf(input, "%d %d\n", width, height) < 2){//a comment
		char c;
		do {
		  c = fgetc(input);
		} while (c != '\n');
		fscanf(input, "%d %d\n", width, height);
	}
	fscanf(input, "%*d\n");

	uint ** rows = malloc(sizeof(uint*)*(*height));
	for(int i = 0; i < *height; i++){
		rows[i] = malloc(sizeof(uint)*(*width));
	}

	for(int i = 0; i < *height; i++){
		for(int j = 0; j < *width; j++){
			fscanf(input, "%u", rows[i]+j);
		}
	}

	free(header);

	return rows;
}

void freeImage(uint ** input, int width, int height){
	for(int i = 0; i < height; i++){
		free(input[i]);
	}
	free(input);
}

uint ** createImage(int width, int height){
	uint ** out = malloc(sizeof(uint*)*height);
	for(int i = 0; i < height; i++){
		out[i] = malloc(sizeof(uint)*(width));
	}
	return out;
}

void saveImage(FILE * output, int width, int height, uint ** input){
	fprintf(output, "P2\n%d %d\n255\n", width, height);
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			if(j != width-1)
				fprintf(output, "%d ", input[i][j]);
			else
				fprintf(output, "%d\n", input[i][j]);
		}
	}
}


int main(int argc, char ** argv){
	int nThreads, partitionType; char * inputFileName, * outputFileName;
	if(argc < 5){
		puts("Invalid number of parameters");
		return 0;
	}
	if((nThreads = atoi(argv[1])) < 1){
		puts("Invalid number of threads");
		return 1;
	}
	if(strcmp(argv[2], "numbers") == 0){
		partitionType = NUMBERS;
	}
	else if(strcmp(argv[2], "blocks") == 0){
		partitionType = BLOCKS;
	}
	else{
		puts("Invalid partiton type.");
		return 0;
	}
	inputFileName = argv[3];
	outputFileName = argv[4];

	FILE * input = fopen(inputFileName, "r");
	if(!input){
		puts("Cannot open input file");
		return 0;
	}
	FILE * output = fopen(outputFileName, "w+");
	if(!output){
		puts("Cannot open oputput file");
		return 0;
	}

	int imageWidth, imageHeight;
	uint ** image = loadImage(input, &imageWidth, &imageHeight);
	if(image == NULL){
		puts("Invalid file header");
		return 0;
	}

	struct timespec beginT, endT;
	clock_gettime(CLOCK_REALTIME, &beginT);


	uint ** newImage = createImage(imageWidth, imageHeight);

	pthread_t * threads = malloc(sizeof(pthread_t)*nThreads);
	jobInfo * jobs = malloc(sizeof(jobInfo)*nThreads);

	for(int i = 0; i < nThreads; i++){
		jobs[i].jobType = partitionType;
		jobs[i].myIndex = i;
		jobs[i].nJobs = nThreads;
		jobs[i].inData = image;
		jobs[i].outData = newImage;
		jobs[i].width = imageWidth;
		jobs[i].height = imageHeight;
		pthread_create(threads+i, NULL, thread_job, (void*)(jobs+i));
	}

	for(int i = 0; i < nThreads; i++){
		pthread_join(threads[i], NULL);
	}

	clock_gettime(CLOCK_REALTIME, &endT);

	long timeTaken = endT.tv_sec*1e6+endT.tv_nsec/1000 - beginT.tv_sec*1e6-beginT.tv_nsec/1000;


	saveImage(output, imageWidth, imageHeight, newImage);
	freeImage(image, imageWidth, imageHeight);
	freeImage(newImage, imageWidth, imageHeight);

	printf("Time taken in microseconds: %ld\n", timeTaken);

	free(threads);
	free(jobs);

	fclose(input);
	fclose(output);
	return 0;
}