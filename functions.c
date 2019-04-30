#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "value.h"
#include "functions.h"

#define MAX_THREADS 30

typedef struct ValueQueueMem {
    Value val;
    struct ValueQueueMem* next;
} ValueQueueMem;
typedef struct ValueQueue {
    struct ValueQueueMem* head;
    struct ValueQueueMem* tail;
} ValueQueue;
void addValueQ(ValueQueue* q, Value v) {
    ValueQueueMem* r = calloc(1, sizeof(ValueQueueMem));
    r->val = v;
    r->next = 0;
    if (q->tail != 0) {
        q->tail->next = r;
    }
    q->tail = r;

    if (q->head == 0) {
        q->head = r;
    }
}
bool isEmptyQ(ValueQueue* q){
    return q->head == 0;
}
Value removeValueQ(ValueQueue* q) {
    ValueQueueMem* r = q->head;
    if (r != 0) {
        q->head = r->next;
        if (q->tail == r) {
            q->tail = 0;
        }
    }
    return r->val;
}
/*
 * Thread Pooling mechanisms
*/
static bool halt = 0;
static ValueQueue* taskQueue = 0;
static ValueQueue* argQueue = 0;
static ValueQueue* resultLocationQueue = 0;
static ValueQueue* readyLocationQueue = 0;
static pthread_mutex_t* taskQueueLock = 0;
static pthread_t availibleThreads[MAX_THREADS];
static size_t numThreads;
static void runOnPool(void* func, Value* args, Value* location, bool* ready){
	//Add the function to the queue of functions to run.
	pthread_mutex_lock(taskQueueLock);
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
			sleep(1);
			continue;
		}
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

struct Function;
typedef struct Function {
    void* exec; //A function that takes in an array of Values, and returns a pointer to a new value.
    size_t numArgs; //Number of arguments.
    ValueQueue** values; //Array of pointers to queues of values we've gotten from the functions we're waiting on.
    struct Notifier* notify; //Head of linked list of other functions to notify when done.
} Function;
typedef struct Notifier {
    Function* notifier; //Function that is notifying.
    Function* listener; //Function doing the listening.
    int index; //The notifier function pipes it's output to the index'th arg of the listener function.
    struct Notifier* next; //For linked list capabilities.
} Notifier;

Function* makeFunction(size_t size, void* func){
    Function* new = calloc(1, sizeof(Function));
    new->exec = func;
    ValueQueue** values = calloc(size, sizeof(ValueQueue*));
    for(int i = 0; i<size; i++){
        ValueQueue* q = calloc(1, sizeof(ValueQueue));
        q->head = 0;
        q->tail = 0;
        values[i] = q;
    }
    new->values = values;
    new->numArgs = size;
    new->notify = 0;
    return new;
}
Notifier* makeNotifier(Function* notifier, Function* listener, size_t index){
    Notifier* new = calloc(1, sizeof(Notifier));
    new->listener = listener;
    new->notifier = notifier;
    new->index = index;
    return new;
}
void waitFor(Function* before, Function* after, int pipeToIndex){
    //Make the notifier.
    Notifier* notifier = makeNotifier(before, after, pipeToIndex);
    //Add it to the "before" function.
    notifier->next=before->notify;
    before->notify=notifier;
}
void notify(Notifier* notifier, Value val){
    if(notifier->index >= 0){ //A negative index signals that we don't pipe output at all.
        addValueQ(notifier->listener->values[notifier->index], val);
    }
    //Check if the function can execute something.
    bool canRun = true;
    for(int i = 0; i<notifier->listener->numArgs; i++){
        if(isEmptyQ(notifier->listener->values[i])){
            //An empty queue. We can't run.
            canRun = false;
            break;
        }
    }
    if(canRun){
        executeFunction(notifier->listener);
    }
}

void executeFunction(Function* function){
    //First, assemble the arguments from each arg queue.
    Value args[function->numArgs];
    for(int i = 0; i<function->numArgs; i++){
        args[i] = removeValueQ((function->values)[i]);
    }
    pthread_t thread;
	volatile Value result = asInt(0);
    volatile bool ready = 0;
    //Run the program in a thread.
	runOnPool(function->exec, args, &result, &ready);
    //Wait until it finishes:
    while(!ready){}
    //Now, it needs to notify all the things that are dependent on it.
    Notifier* notifier = function->notify;
    while(notifier != 0){
        //Tell the next function that we're done, here's our value, do whatever you want with it
        notify(notifier, result);
        notifier = notifier->next;
    }
}



