#ifndef STRING_CHECKER
#define STRING_CHECKEr
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t digit(char c){
	return (int)(c - '0');
}

typedef enum{
	FIRST_GREATER,
	SECOND_GREATER,
	EQUAL
}compareResult;

compareResult compare(char * number1, char * number2){
	size_t len1 = strlen(number1);
	size_t len2 = strlen(number2);
	if(len1 > len2)
		return FIRST_GREATER;
	else if(len1 < len2)
		return SECOND_GREATER;
	else{
		for(int index = 0; index < len1; index++){
			if(number1[index] > number2[index])
				return FIRST_GREATER;
			else if(number1[index] < number2[index])
				return SECOND_GREATER;
		}
	}
	return EQUAL;//equal
}


char * addTwoDigitStrings(char * string1, char * string2){
	if(strlen(string1) < strlen(string2)){
		char * temp = string1;
		string1 = string2;
		string2 = temp;
	}

	size_t len1 = strlen(string1);
	size_t len2 = strlen(string2);
	uint8_t * result = calloc(sizeof(uint8_t), len1+1);
	uint8_t carry = 0;
	for(int i = len2-1; i >= 0; i--){
		uint8_t sum = digit(string1[len1-len2+i])+digit(string2[i])+carry;
		result[len2-1-i] = sum % 10;
		carry = sum / 10;
	}
	for(int i = len1-len2-1; i>= 0; i--){
		uint8_t sum = digit(string1[i])+carry;
		result[len1-i-1] = sum % 10;
		carry = sum / 10;
	}

	if(carry)
		result[len1] = carry;

	int index = len1;
	while(index >= 0 && result[index] == 0){
		index--;
	}

	char * res;
	if(index==-1){
		res = calloc(sizeof(char),2);
		res[0] = '0';
	}
	else{
		res = calloc(sizeof(char), index+2);
		for(int i = 0; i <= index; i++){
			res[i] = '0'+result[index-i];
		}
	}


	free(result);
	return res;
}

char * halveDigitString(char * string){
	size_t len = strlen(string);
	uint8_t * result = calloc(sizeof(uint8_t), len+1);
	uint8_t carry = 0;
	for(size_t i = 0; i < len; i++){
		result[i] = (carry+digit(string[i]))/2;
		carry = (carry+digit(string[i])) % 2;
		carry *= 10;
	}
	int index = 0;
	while(index < len && result[index] == 0)
		index++;
	char * res;
	if(index == len){
		res = malloc(sizeof(char)*2);
		res[0] = '0';
		res[1] = 0;
	}
	else{
		res = calloc(sizeof(char), len-index+1);
		for(int i = index; i < len; i++)
			res[i-index] = result[i] + '0';
	}
	free(result);
	return res;
}

char * digitStringMultiply(char * string1, char * string2){
	size_t len1 = strlen(string1), len2 = strlen(string2);
	//find smaller string
	size_t biggerSize = len1, smallerSize = len2;
	char * bigger = string1, * smaller = string2;
	if (len1 < len2){
		bigger = string2;
		smaller = string1;
		biggerSize = len2;
		smallerSize = len1;
	}

	int8_t * result = calloc(sizeof(int8_t), len1+len2);
	memset(result, 0, sizeof(int8_t)*(len1+len2));

	for(int indexBigger = biggerSize-1; indexBigger >= 0; indexBigger--){
		int carry = 0;
		int biggerNum = digit(bigger[indexBigger]);
		for(int indexSmaller = smallerSize-1; indexSmaller >= 0; indexSmaller--){
			size_t resultIndex = biggerSize-1-indexBigger + smallerSize-1-indexSmaller;
			int smallerNum = digit(smaller[indexSmaller]);
			int sum = smallerNum*biggerNum + carry + result[resultIndex];
			carry = sum/10;
			result[resultIndex] = sum % 10;
		}
		if(carry > 0){
			size_t carryIndex = biggerSize-1-indexBigger+smallerSize;
			result[carryIndex] += carry;
		}
	}

	int parserIndex = len1+len2-1;
	while(parserIndex >= 0 && result[parserIndex] == 0)
		parserIndex--;

	char * res;
	if(parserIndex == -1){
		res = calloc(sizeof(char), 2);
		res[0] = '0';
	}
	else{
		res = calloc(sizeof(char), parserIndex+2);
		size_t oIndex = 0;
		while(parserIndex >= 0)
			res[oIndex++] = result[parserIndex--] + '0';
	}
	free(result);
	return res;
}


char * digitSqrt(char * string){
	char * d = calloc(sizeof(char*), 2);
	d[0] = '1';
	char * ds = digitStringMultiply(d, d);
	char * a = calloc(sizeof(char*), 2);
	a[0] = '0';
	while(compare(ds, string) != FIRST_GREATER){
		char* tofree1 = d, *tofree2=ds;
		d = addTwoDigitStrings(d, d);
		ds = digitStringMultiply(d, d);
		free(tofree1);
		free(tofree2);
	}
	//while N < (a+d)^2
	char * sum = addTwoDigitStrings(a, d);
	char * sumsq = digitStringMultiply(sum, sum);
	while(compare(d, "1") != EQUAL){
		char * tofree;
		tofree = d;
		d = halveDigitString(d);
		free(tofree);
		tofree = sum;
		sum = addTwoDigitStrings(d, a);
		free(tofree);
		tofree = sumsq;
		sumsq = digitStringMultiply(sum, sum);
		free(tofree);
		if(compare(string, sumsq) != SECOND_GREATER){
			tofree = a;
			a = addTwoDigitStrings(a, d);
			free(tofree);
		}
	}
	free(d);
	free(sum);
	free(sumsq);
	return a;
}


#ifdef CHECKER_TEST
//gcc -DCHECKER_TEST -x c stringchecker.h  -o checker.out 

int main(int argc, char ** argv){
	char digit1[] = "123123787125190951251";
	char digit2[] = "12581251812391230120312312312";
	char Expected[] = "1549051369917280660322943402312107099546679102312";
	char * result = digitStringMultiply(digit1, digit2);
	puts("Expected:");
	puts(Expected);
	puts("Got:");
	puts(result);
	if(compare(Expected, result) == EQUAL){
		puts("Result equal");
	}
	free(result);

	puts("Halving test");
	char digit3[] = "1248912512512523";
	result = halveDigitString(digit3);
	char Expected2[] = "624456256256261";
	puts("Expected:");
	puts(Expected2);
	puts("Got:");
	puts(result);
	if(compare(Expected2, result) == EQUAL){
		puts("Result equal");
	}
	free(result);

	puts("Adding test");
	char Expected3 [] = "12581251935515017245503263563";
	puts("Expected:");
	puts(Expected3);
	result = addTwoDigitStrings(digit1, digit2);
	puts("Got:");
	puts(result);
	if(compare(Expected3, result) == EQUAL){
		puts("Result equal");
	}
	free(result);

	char digit4[] = "479374052671704457744";
	puts("Sqrt test");
	char Expected4 [] = "21894612412";
	puts("Expected:");
	puts(Expected4);
	result = digitSqrt(digit4);
	puts("Got:");
	puts(result);
	if(compare(Expected4, result) == EQUAL){
		puts("Result equal");
	}
	free(result);

	return 0;
}
#endif

#endif