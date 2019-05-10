//
// Created by jack on 5/9/2019.
//

#ifndef COMPILER_LIB_H
#define COMPILER_LIB_H

#include "functions.h"
#include "pool.h"
#include "worker.h"
#include "atomic.h"
void store(Value* args){
	if(args[1].asPointer == NULL){
		return;
	}
	*((void**) args[1].asPointer) = args[0].asPointer;
}
Function* makeStoreFunction(void** addr){
	Function* newFunc = makeFunction(1, store);
	setArg(newFunc, 1, asPointer(addr));
	return newFunc;
}
void totalSync(){
	if(_SAFE_MODE_){
		localSafeSync();
	}
	else {
		localWorkerSync();
	}
}

#endif //COMPILER_LIB_H
