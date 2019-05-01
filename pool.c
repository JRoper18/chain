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
#include "functions.h"

#define MAX_THREADS 30

/*
 * Thread Pooling mechanisms
*/
static bool halt = 0;
static ValueQueue* taskQueue = 0;
static ValueQueue* argQueue = 0;
static int queueSize = 0;
static pthread_mutex_t* taskQueueLock = 0;
static pthread_t availibleThreads[MAX_THREADS];
static pthread_mutex_t finishLocks[MAX_THREADS];
static size_t numThreads;
void runOnPool(Function* func, Value* args){
	//Add the function to the queue of functions to run.
	pthread_mutex_lock(taskQueueLock);
	queueSize++;
	addValueQ(taskQueue, asPointer(func));
	addValueQ(argQueue, asPointer(args));
	pthread_mutex_unlock(taskQueueLock);
}
static void pooledThread(int index){
	while(!halt){
		pthread_mutex_lock(taskQueueLock);
		if(isEmptyQ(taskQueue) || isEmptyQ(argQueue)){
			//Wait a sec, there's nothing to run.
			pthread_mutex_unlock(taskQueueLock);
			sleep(0.1);
			continue;
		}
		pthread_mutex_lock(&finishLocks[index]);
		queueSize--;
		Value funcValue = removeValueQ(taskQueue);
		Value argValue = removeValueQ(argQueue);
		pthread_mutex_unlock(taskQueueLock);
		Function* function = (Function*) funcValue.asPointer;
		Value (*actualFunc)(Value*) = function->exec;
		Value* args = (Value*) argValue.asPointer;
		//Run the function and get the value.
		Value result;
		if(function->numArgs == 0){
			result = actualFunc(NULL);
		}
		else {
			result = actualFunc(args);
		}
		//The args have been used, so we can free them.
		free(args);
		//Now, it needs to notify all the things that are dependent on it.
		Notifier* notifier = function->notify;
		while(notifier != 0){
			//Tell the next function that we're done, here's our value, do whatever you want with it
			notify(notifier, result);
			notifier = notifier->next;
		}
		pthread_mutex_unlock(&finishLocks[index]);
	}
	pthread_exit(NULL);
}
void initPool(){
	taskQueue = calloc(1, sizeof(ValueQueue));
	argQueue = calloc(1, sizeof(ValueQueue));
	taskQueueLock = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(taskQueueLock, NULL) != 0){
		printf("\n mutex init failed\n");
		exit(0);
	}
	//Start out with a number of threads equal to the number of cores in our CPU.
	numThreads = sysconf(_SC_NPROCESSORS_ONLN);
	for(int i = 0; i<numThreads; i++){
		pthread_create(&availibleThreads[i], NULL, (void*) pooledThread, i);
		if (pthread_mutex_init(&finishLocks[i], NULL) != 0){
			printf("\n mutex init failed\n");
			exit(0);
		}
	}
}
void sync(){
	//Lock all the finish locks in order to ensure that all the threads are done with tasks and that they don't take any more.
	for(int i = 0; i<numThreads; i++){
		pthread_mutex_lock(&finishLocks[i]);
	}
	//Alright, we're synced.
	for(int i = 0; i<numThreads; i++){
		pthread_mutex_unlock(&finishLocks[i]);
	}

}