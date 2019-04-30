//
// Created by jack on 4/30/2019.
//

#ifndef COMPILER_POOL_H
#define COMPILER_POOL_H

#include "value.h"

void runOnPool(void* func, Value* args, Value* location, bool* ready);
void initPool();
#endif //COMPILER_POOL_H
