//
// Created by jack on 4/25/2019.
//

#ifndef COMPILER_FUNCTIONS_H
#define COMPILER_FUNCTIONS_H
#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include "pool.h"

typedef struct ValueQueueMem ValueQueueMem;
typedef struct ValueQueue ValueQueue;
extern void addValueQ(ValueQueue* q, Value v);
extern bool isEmptyQ(ValueQueue* q);
extern Value removeValueQ(ValueQueue* q);

struct Function;
typedef struct Function Function;
typedef struct Notifier Notifier;

extern void executeFunction(Function* function);

extern Function* makeFunction(size_t size, void* func);
extern Notifier* makeNotifier(Function* notifier, Function* listener, size_t index);
extern void waitFor(Function* before, Function* after, int pipeToIndex);
extern void notify(Notifier* notifier, Value val);
extern void executeFunction(Function* function);

#endif //COMPILER_FUNCTIONS_H
