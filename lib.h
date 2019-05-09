//
// Created by jack on 5/9/2019.
//

#ifndef COMPILER_LIB_H
#define COMPILER_LIB_H

#include "functions.h"
#include "pool.h"
#include "worker.h"
#include "atomic.h"

void totalSync(){
	if(_SAFE_MODE_){
		localSafeSync();
	}
	else {
		localWorkerSync();
	}
}

#endif //COMPILER_LIB_H
