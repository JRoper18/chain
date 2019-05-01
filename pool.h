//
// Created by jack on 4/30/2019.
//

#ifndef COMPILER_POOL_H
#define COMPILER_POOL_H

#include "value.h"
#include "functions.h"

void runOnPool(Function* func, Value* args);
void initPool();
void sync();
#endif //COMPILER_POOL_H
