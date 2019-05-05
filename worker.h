//
// Created by jack on 5/4/2019.
//

#ifndef COMPILER_WORKER_H
#define COMPILER_WORKER_H

#include "functions.h"

typedef struct FunctionWrapper {
	Function* func;
	struct FunctionWrapper* prev;
	struct FunctionWrapper* next;
} FunctionWrapper;
typedef struct TaskQ {
	struct FunctionWrapper* head;
	struct FunctionWrapper* tail
} TaskQ;

#endif //COMPILER_WORKER_H
