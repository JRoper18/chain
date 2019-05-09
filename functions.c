#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "value.h"
#include "functions.h"
#include "pool.h"
#include "worker.h"

bool _SAFE_MODE_ = 0;

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
        executeFunction(notifier->listener, NULL);
    }
}

void executeFunction(Function* function, Value* argsIn){
	Value* args;
	if(argsIn != NULL || function->numArgs == 0){
		args = argsIn;
	}
	else {
		args = calloc(function->numArgs, sizeof(Value));
		//First, assemble the arguments from each arg queue.
		for(int i = 0; i<function->numArgs; i++){
			if(function->set[i]){ //If the set bit is true, it's not a pointer to a value queue, it's the permanent value in that arg.
				args[i] = asPointer(function->values[i]);
			}
			else{
				args[i] = removeValueQ((function->values)[i]);
			}
		}
	}
	if(_SAFE_MODE_){
		runOnPool(function, args);
	}
	else {
		addTask(function, args);
	}
}


Function* cloneFunction(Function* old, bool cloneNofifiers){
	int size = old->numArgs;
	if(cloneNofifiers){
		Function* newFunc = calloc(1, sizeof(Function));
		newFunc->numArgs = size;
		newFunc->exec = old->exec;
		newFunc->notify = old->notify;
		for(int i = 0; i<size; i++){
			newFunc->set[i] = old->set[i];
			if(newFunc->set[i]){
				newFunc->values[i] = old->values[i];
			}
			else{
				//Clone each queue.
				ValueQueue* newQ = calloc(1, sizeof(ValueQueue));
				ValueQueue* oldQ = old->values[i];
				while(!isEmptyQ(oldQ)){
					addValueQ(newQ, removeValueQ(oldQ));
				}
				newFunc->values[i] = newQ;
			}
		}
		return newFunc;
	}
	return makeFunction(size, old->exec);
}
