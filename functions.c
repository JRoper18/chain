#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "value.h"
#include "functions.h"
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

void executeFunction(Function* function);

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
    void* threadResult;
    //Run the program in a thread.
    pthread_create(&thread, NULL, function->exec, args);
    //Wait until it finishes:
    int joinSuccess = pthread_join(thread, &threadResult);
    if(joinSuccess != 0){
        printf("Fatal error waiting on function!");
        exit(0);
    }
    Value ret = (Value) threadResult;
    //Now, it needs to notify all the things that are dependent on it.
    Notifier* notifier = function->notify;
    while(notifier != 0){
        //Tell the next function that we're done, here's our value, do whatever you want with it
        notify(notifier, ret);
        notifier = notifier->next;
    }
}


/*
 * Thread Pooling mechanisms
 */
pthread_t* availibleThreads;
size_t numThreads;
const int MAX_THREADS = 30;
void runAndReturn(void* func, Value* args){

}
