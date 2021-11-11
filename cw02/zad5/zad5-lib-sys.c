#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef SYSFUN
	bool breaker(char * inputPath, char * outputPath){
		int distanceSinceLastLine = 0;
		int in = open(inputPath, O_RDONLY);
		int out = open(outputPath, O_CREAT | O_TRUNC | O_WRONLY | O_NONBLOCK, 0664);
		if(in == -1 || out == -1)
			return false;
		char i;
		while(read(in, &i, 1) > 0){
			printf("%c", in);
			if(i == '\n')
				distanceSinceLastLine = -1;
			else if(distanceSinceLastLine > 49){
				write(out, "\n", 1);
				distanceSinceLastLine = 0;
			}
			write(out, &i, 1);

			distanceSinceLastLine++;
		}
		close(in);
		close(out);
		return true;
	}
#else
	bool breaker(char * inputPath, char * outputPath){
		int distanceSinceLastLine = 0;

		FILE * in = fopen(inputPath, "r");
		FILE * out = fopen(outputPath, "w");
		if(!in || !out)
			return false;
		char i;
		while(fread(&i, sizeof(char), 1, in) > 0){
			if(i == '\n')
				distanceSinceLastLine = -1;
			else if(distanceSinceLastLine > 49){
				fwrite("\n", sizeof(char), 1, out);
				distanceSinceLastLine = 0;
			}
			fwrite(&i, sizeof(char), 1, out);
			distanceSinceLastLine++;
		}
		fclose(in);
		fclose(out);
		return true;
	}
#endif

int main(int argc, char ** argv){
	if(argc < 3){
		puts("Usage: program <input file> <output file>");
		return 0;
	}
	if(!breaker(argv[1], argv[2]))
		puts("Unable to access one of the files!");
	return 0;
}