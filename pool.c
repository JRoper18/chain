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
#include "worker.h"

#define MAX_THREADS 30

/*
 * Thread Pooling mechanisms
*/
volatile static bool wait = 0;
static bool end = 0;
volatile int numRun = 0;
volatile int numAdded = 0;
volatile ValueQueue* taskQueue = 0;
volatile ValueQueue* argQueue = 0;
volatile ValueQueue* taskLevelQueue = 0;
volatile static int queueSize = 0;
static pthread_mutex_t taskQueueLock;
static pthread_t availibleThreads[MAX_THREADS];
static int threadLevels[MAX_THREADS];
static int taskLevelsRemaining[MAX_LEVEL];
static pthread_barrier_t syncBarrier;
static size_t numThreads;
void runOnPool(Function* func, Value* args){
	//Add the function to the queue of functions to run.
	pthread_mutex_lock(&taskQueueLock);
	numAdded++;
	queueSize++;
	addValueQ(taskQueue, asPointer(func));
	addValueQ(argQueue, asPointer(args));
	pthread_mutex_unlock(&taskQueueLock);
}
static void pooledThread(int index){
	while(!end){
		pthread_mutex_lock(&taskQueueLock);
		if(taskQueue->head == 0){
			//Nothing to run.
			pthread_mutex_unlock(&taskQueueLock);
			if(wait){ //We will only wait if there's no more jobs to do.
				pthread_barrier_wait(&syncBarrier);
			}
			continue;
		}
		queueSize--;
		numRun++;
		Value funcValue = removeValueQ(taskQueue);
		Value argValue = removeValueQ(argQueue);
		pthread_mutex_unlock(&taskQueueLock);
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
		//Now, it needs to notify all the things that are dependent on it.
		Notifier* notifier = function->notify;
		if(notifier != 0){
			while(notifier != 0){
				Value* args = calloc(notifier->listener->numArgs, sizeof(Value));
				pthread_mutex_lock(&taskQueueLock);
				//Tell the next function that we're done, here's our value, do whatever you want with it
				if(notifier->index >= 0){ //A negative index signals that we don't pipe output at all.
					addValueQ(notifier->listener->values[notifier->index], result);
				}
				//Check if the function can execute something.
				bool canRun = true;
				//First, assemble the arguments from each arg queue.
				for(int i = 0; i<notifier->listener->numArgs; i++){
					if(function->set[i]){ //If the set bit is true, it's not a pointer to a value queue, it's the permanent value in that arg.
						args[i] = asPointer(notifier->listener->values[i]);
					}
					else{

						if(isEmptyQ(notifier->listener->values[i])){
							//Don't have all the arguments
							canRun = false;
							free(args);
							break;
						}
						Value arg = removeValueQ(notifier->listener->values[i]);
						args[i] = arg;
					}
				}
				pthread_mutex_unlock(&taskQueueLock);
				if(canRun){
					//Add the task to the queue.
					runOnPool(notifier->listener, args);
				}
				notifier = notifier->next;
			}
		}
	}
	pthread_exit(NULL);
}
void initPool(){
	taskQueue = calloc(1, sizeof(ValueQueue));
	argQueue = calloc(1, sizeof(ValueQueue));
	if (pthread_mutex_init(&taskQueueLock, NULL) != 0){
		printf("\n mutex init failed\n");
		exit(0);
	}
	//Start out with a number of threads equal to the number of cores in our CPU.
	numThreads = 3; //sysconf(_SC_NPROCESSORS_ONLN);
	if(pthread_barrier_init(&syncBarrier, NULL, numThreads + 1) != 0){
		printf("\n barrier init failed\n");
		exit(0);
	}
	for(int i = 0; i<numThreads; i++){
		pthread_create(&availibleThreads[i], NULL, (void*) pooledThread, i);
	}
}

void localSafeSync(){
	wait = 1;
	//Now, we wait until we can pass the barrier.
	pthread_barrier_wait(&syncBarrier);
	//Now, if there's anything in the task queue, it needs to finish, so repeat until that's the case.
	wait = 0;
	/*
	while(true){
		bool remaining;
		for(int i = 0; i<numThreads; i++){
		}
		if(!remaining){
			break;
		}
	}*/
}
void finish(){
	wait = 1;
	//Now, we wait until we can pass the barrier.
	pthread_barrier_wait(&syncBarrier);
	//Now, if there's anything in the task queue, it needs to finish, so repeat until that's the case.
	wait = 0;
	end = 1;
}