#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "value.h"
#include "functions.h"
#include "pool.h"

struct Function;
typedef struct Function {
    void* exec; //A function that takes in an array of Values, and returns a pointer to a new value.
    size_t numArgs; //Number of arguments.
    bool* set; //Array of which valuequeues are just permanently-set values.
    ValueQueue** values; //Array of pointers to queues of values we've gotten from the functions we're waiting on.
    struct Notifier* notify; //Head of linked list of other functions to notify when done.
} Function;
typedef struct Notifier {
    Function* listener; //Function doing the listening.
    int index; //The notifier function pipes it's output to the index'th arg of the listener function.
    struct Notifier* next; //For linked list capabilities.
} Notifier;

Function* makeFunction(size_t size, void* func){
    Function* new = calloc(1, sizeof(Function));
    new->exec = func;
    ValueQueue** values = calloc(size, sizeof(ValueQueue*));
    bool* set = calloc(size, sizeof(bool));
    for(int i = 0; i<size; i++){
        ValueQueue* q = calloc(1, sizeof(ValueQueue));
        set[i] = 0;
        q->head = 0;
        q->tail = 0;
        values[i] = q;
    }
    new->set = set;
    new->values = values;
    new->numArgs = size;
    new->notify = 0;
    return new;
}
Notifier* makeNotifier(Function* listener, size_t index){
    Notifier* new = calloc(1, sizeof(Notifier));
    new->listener = listener;
    new->index = index;
    return new;
}
void setArg(Function* function, size_t argNum, Value in){
	function->set[argNum] = 1;
	function->values[argNum] = in.asPointer;
}
void waitFor(Function* before, Function* after, int pipeToIndex){
    //Make the notifier.
    Notifier* notifier = makeNotifier(after, pipeToIndex);
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
    	ValueQueue* queue = notifier->listener->values[i];
    	bool set = notifier->listener->set[i];
    	if(isEmptyQ(queue) && !set){
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
    	if(function->set[i]){ //If the set bit is true, it's not a pointer to a value queue, it's the permanent value in that arg.
    		args[i] = asPointer(function->values[i]);
    	}
    	else{
			args[i] = removeValueQ((function->values)[i]);
    	}
    }
    pthread_t thread;
	Value result = asInt(0);
    bool ready = 0;
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



