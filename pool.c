//
// Created by jack on 4/30/2019.
//

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "pool.h"
#include "value.h"

#define MAX_THREADS 30

/*
 * Thread Pooling mechanisms
*/
static bool halt = 0;
static ValueQueue* taskQueue = 0;
static ValueQueue* argQueue = 0;
static ValueQueue* resultLocationQueue = 0;
static ValueQueue* readyLocationQueue = 0;
static int queueSize = 0;
static pthread_mutex_t* taskQueueLock = 0;
static pthread_t availibleThreads[MAX_THREADS];
static size_t numThreads;
void runOnPool(void* func, Value* args, Value* location, bool* ready){
	//Add the function to the queue of functions to run.
	pthread_mutex_lock(taskQueueLock);
	queueSize++;
	addValueQ(taskQueue, asPointer(func));
	addValueQ(argQueue, asPointer(args));
	addValueQ(resultLocationQueue, asPointer(location));
	addValueQ(readyLocationQueue, asPointer(ready));
	pthread_mutex_unlock(taskQueueLock);
}
static void pooledThread(){
	while(!halt){
		pthread_mutex_lock(taskQueueLock);
		if(isEmptyQ(taskQueue)){
			//Wait a sec, there's nothing to run.
			pthread_mutex_unlock(taskQueueLock);
			sleep(0.1);
			continue;
		}
		queueSize--;
		Value funcValue = removeValueQ(taskQueue);
		Value argValue = removeValueQ(argQueue);
		Value* returnLocation = (Value*) removeValueQ(resultLocationQueue).asPointer;
		bool* readyLocation = removeValueQ(readyLocationQueue).asPointer;
		pthread_mutex_unlock(taskQueueLock);
		Value (*nextFunc)(Value*) = funcValue.asPointer;
		Value* args = (Value*) argValue.asPointer;
		//Run the function and get the value.
		Value result = nextFunc(args);
		//Put the return value in the corresponding result location.
		*returnLocation = result;
		//Also, put the ready bit in the ready location.
		*readyLocation = 1;
	}
	pthread_exit(NULL);
}
void initPool(){
	taskQueue = calloc(1, sizeof(ValueQueue));
	argQueue = calloc(1, sizeof(ValueQueue));
	resultLocationQueue = calloc(1, sizeof(ValueQueue));
	readyLocationQueue = calloc(1, sizeof(ValueQueue));
	taskQueueLock = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(taskQueueLock, NULL) != 0){
		printf("\n mutex init failed\n");
		exit(0);
	}
	//Start out with a number of threads equal to the number of cores in our CPU.
	numThreads = sysconf(_SC_NPROCESSORS_ONLN);
	for(int i = 0; i<numThreads; i++){
		pthread_create(&availibleThreads[i], NULL, (void*) pooledThread, NULL);
	}
}