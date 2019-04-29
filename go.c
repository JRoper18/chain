#include "go.h"
#include <stdlib.h>
#include <stdio.h>

struct Routine;
typedef struct Routine Routine;
extern int main();
void magic(Routine* future){
	asm("    push %rbx\n"
		"    push %rbp\n"
		"    push %r12\n"
		"    push %r13\n"
		"    push %r14\n"
		"    push %r15\n"
		"\n"
		"    mov %rdi,%r15\n"
		"\n"
		"    call _current\n"
		"\n"
		"    mov (%rax),%rbx\n"
		"    mov %r15,(%rax)\n"
		"    mov %rsp,(%rbx)\n"
		"    mov (%r15),%rsp\n"
		"\n"
		"    pop %r15\n"
		"    pop %r14\n"
		"    pop %r13\n"
		"    pop %r12\n"
		"    pop %rbp\n"
		"    pop %rbx\n"
		"\n"
		"    ret");
};
#define STACK_ENTRIES (8192 / sizeof(uint64_t))

typedef struct Queue {
    struct Routine* head;
    struct Routine* tail;
} Queue;

struct Channel {
    bool poisoned;
    Queue* sending;
    Queue* receiving;
    /* more */
};

struct Routine {
    uint64_t saved_rsp;
    uint64_t starting_rsp;
    Routine *next;
    Channel* channel;
    uint64_t func;
    Value val;
    /* more */
};

Routine* nextSender(Channel* c){
    return c->sending->head;
}
Routine* nextReceiver(Channel* c){
    return c->receiving->head;
}
/////////

void addQ(Queue* q, Routine* r) {
    r->next = 0;
    if (q->tail != 0) {
        q->tail->next = r;
    }
    q->tail = r;

    if (q->head == 0) {
        q->head = r;
    }
}

Routine* removeQ(Queue* q) {
    Routine* r = q->head;
    if (r != 0) {
        q->head = r->next;
        if (q->tail == r) {
            q->tail = 0;
        }
    }
    return r;
}

///////////////////////////////////////////////////
Routine *current_ = 0;
Queue ready = { 0, 0};
Queue zombies = { 0, 0 };


Routine** current() {
    if (current_ == 0) {
        //Create a routine for main here.

        Routine* mainRoutine = malloc(sizeof(Routine));
        Channel* mainChannel = malloc(sizeof(Channel));
        mainChannel->sending = malloc(sizeof(Queue));
        mainChannel->receiving = malloc(sizeof(Queue));
        mainRoutine->channel = mainChannel;
        mainRoutine->func = (uint64_t) main;
        current_ = (mainRoutine);
        magic(mainRoutine);
    }
    return &current_;
}

/* OSX is stuck in the past and prepends _ in front of external symbols */
Routine** _current() {
    return current();
}


/**
        This function will be called when any routine ends. It will add the routine to the zombie queue. .
*/
void endRoutine(){
    addQ(&zombies, *current()); //Add the routine to the zombie queue.
    //Go to the next availible routine.
    Routine* next = removeQ(&ready);
    if(next == 0){
        //Crud. No more ready routines.
        exit(0);
    }
    magic(next);
}
Channel* go(Func func) {
    //Create a new routine for the function. When it exits/blocks, it'll return to this function.
    Channel* routChannel = malloc(sizeof(Channel));
    routChannel->sending = malloc(sizeof(Queue));
    routChannel->receiving = malloc(sizeof(Queue));
    uint64_t* newStack = malloc((STACK_ENTRIES) * sizeof(uint64_t));
    newStack += STACK_ENTRIES; //Go to the end of the stack.
    Func* wrapperFuncLocation = (Func*) newStack;
    *(wrapperFuncLocation) = endRoutine;
    newStack--; //Go down to the actual function.
    //Now, we want the function to point to the new call stack.
    uint64_t* functionLocation = newStack;
    *(functionLocation) = (uint64_t) func;
    newStack -= 6; //Give us 6 extra spaces for input registers, AND it's MINUS 6 because the stack expands downwards.
    Routine* newRoutine = malloc(sizeof(Routine));
    newRoutine->saved_rsp = (uint64_t) newStack;
    newRoutine->starting_rsp = (uint64_t) newStack;
    newRoutine->func = (uint64_t) func;
    newRoutine->channel = routChannel;
    //Add the routine to the queue.
    addQ(&ready, newRoutine);
    return routChannel;
}
Channel* me() {
    return (*current())->channel;
}

void again() {
    //Create the new routine.
    uint64_t* newStack = malloc((STACK_ENTRIES) * sizeof(uint64_t));
    newStack += STACK_ENTRIES; //Go to the end of the stack.
    Func* wrapperFuncLocation = (Func*) newStack;
    *(wrapperFuncLocation) = endRoutine;
    newStack--; //Go down to the actual function.
    //Now, we want the function to point to the new call stack.
    uint64_t* functionLocation = newStack;
    uint64_t oldFuncLoc = (*current())->func;
    *(functionLocation) = oldFuncLoc;
    newStack -= 6; //Give us 6 extra spaces for input registers, AND it's MINUS 6 because the stack expands downwards.
    Routine* newRoutine = malloc(sizeof(Routine));
    newRoutine->saved_rsp = (uint64_t) newStack;
    newRoutine->channel = (*current())->channel;
    newRoutine->func = (*current())->func;
    //Now, add the new routine to the head of the queue.
    addQ(&zombies, *current()); //Add the routine to the zombie queue.
    //Run the new routine.
    magic(newRoutine);
}

Channel* channel() {
    Channel* newChannel = malloc(sizeof(Channel));
    newChannel->sending = malloc(sizeof(Queue));
    newChannel->receiving = malloc(sizeof(Queue));
    newChannel->poisoned = false;
    (*current())->channel = newChannel;
    return newChannel;
}

bool isPoisoned(Channel* ch) {
    return ch->poisoned;
}

void poison(Channel* ch) {
    ch->poisoned = true;
}

Value receive(Channel* ch) {
    if(isPoisoned(ch)){
        endRoutine();
    }
    addQ(ch->receiving, *current()); //Make us the current reciever of it.
    //Alright. Does the channel have a value for us?
    while(true){
        if(nextSender(ch) != 0){
            //We have a sender!
            Routine* sender = nextSender(ch);
            //We're going to continue on the receiver path, so make sure the sender is marked as ready.
            addQ(&ready, sender);
            removeQ(ch->sending);
            removeQ(ch->receiving); //Reset the sender and reciever because they matched and the value was sent.
            return (sender->val);
        }
        else {
            Routine* next = removeQ(&ready);
            if(next == 0){ //No more routines to go through. We're done.
                exit(0);
            }
            magic(next);
        }
    }
}

void send(Channel* ch, Value v) {
    if(isPoisoned(ch)){
        endRoutine();
    }
    //Mark that the channel has a sender.
    addQ(ch->sending, *current());
    (*current())->val = v;
    //Put the value in the channel.
    if(nextReceiver(ch) != 0){
        Routine* receiver = nextReceiver(ch);
        //Go to the reciever, if there is one.
        magic(receiver);
    }
    else{
        //Nothing is receiving, so go to the next routine.
        Routine* next = removeQ(&ready);
        if(next == 0){
            //No ready routines.
            exit(0);
        }
        magic(next);
    }

}
////////////
// Stream //
////////////

struct Stream {
    Channel* ch;
};

static void streamLogic(void) {
    StreamFunc fs = (StreamFunc) receive(me()).asPointer;
    Value v = receive(me());
    fs(v);
    poison(me());
}

Stream* stream(StreamFunc func, Value v) {
    Channel* ch = go(streamLogic);
    send(ch, asPointer(func));
    send(ch,v);
    Stream* out = (Stream*) calloc(sizeof(Stream),1);
    out->ch = ch;
    return out;
}

Value next(Stream* s) {
    return receive(s->ch);
}

bool endOfStream(Stream* s) {
    return isPoisoned(s->ch);
}

void yield(Value v) {
    send(me(),v);
}
