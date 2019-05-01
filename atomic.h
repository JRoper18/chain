//
// Created by jack on 4/26/2019.
//

#ifndef COMPILER_ATOMIC_H
#define COMPILER_ATOMIC_H
//
// Created by jack on 4/26/2019.
//
#include <pthread.h>
#include "functions.h"
#include "value.h"

typedef struct Atomic {
    Value val;
    pthread_mutex_t lock;
} Atomic;
Value deltaAtomic(Atomic* atomic, Value func(Value));
void incrementAtomic(Atomic* atomic, int amount);
void setAtomic(Atomic* atomic, Value val);
Value getAtomic(Atomic* atomic);
Atomic* makeAtomic(Value val);
#endif //COMPILER_ATOMIC_H
