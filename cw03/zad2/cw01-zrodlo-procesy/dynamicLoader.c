#include "dynamicLoader.h"
#include <stdio.h>

DYNAMIC_IMPORT(MAKE_POINTERS);


void * dlHandle;

void dynamicLoad(){
	dlHandle = dlopen("libBlockTable.so.1", RTLD_LAZY);
	if(dlHandle == NULL)
		puts("NULL HANDLE! CHECK LD PRELOAD");
	DYNAMIC_IMPORT(LOAD_POINTERS);
}

void dynamicClose(){
	dlclose(dlHandle);
}