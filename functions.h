//
// Created by jack on 4/25/2019.
//

#ifndef COMPILER_FUNCTIONS_H
#define COMPILER_FUNCTIONS_H
#include <stdio.h>
#include <stdlib.h>
#include "value.h"

typedef struct ValueQueueMem ValueQueueMem;
typedef struct ValueQueue ValueQueue;
extern void addValueQ(ValueQueue* q, Value v);
extern bool isEmptyQ(ValueQueue* q);
extern Value removeValueQ(ValueQueue* q);

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
typedef struct Notifier Notifier;

extern void executeFunction(Function* function, Value* args);
extern Function* makeFunction(size_t size, void* func);
extern Notifier* makeNotifier(Function* listener, size_t index);
extern void waitFor(Function* before, Function* after, int pipeToIndex);
extern void notify(Notifier* notifier, Value val);
extern void executeFunction(Function* function, Value* args);
void setArg(Function* function, size_t argNum, Value in);
#endif //COMPILER_FUNCTIONS_H
