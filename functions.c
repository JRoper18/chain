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



