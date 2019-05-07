//
// Created by jack on 5/4/2019.
//

#ifndef COMPILER_WORKER_H
#define COMPILER_WORKER_H

#include "functions.h"

#define MAX_LEVEL 100

typedef struct Task {
	Function* func;
	Value* args;
	int spawningId;
	int level;
	struct Task* next;
} Task;
typedef struct TaskQ {
	volatile int deepestLevel;
	volatile int tasksRemaining[MAX_LEVEL];
	struct Task* levels[MAX_LEVEL];
} TaskQ;
void addTask(Function* task, Value* args);
void localSync();
void makeWorkers();
void finishAllWorkers();
#endif //COMPILER_WORKER_H
