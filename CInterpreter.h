#ifndef C_INTERPRETER_H
#define C_INTERPRETER_H

#include <stdbool.h>

typedef struct params
{
    char* type;
    char* name;
    char** funcs;
    bool isAtomic;
} params;

typedef struct functionInfo
{
    char* modifiers;
    char* name;
    params* parameters;
} functionInfo;

extern char* findLastWord (char*);
extern functionInfo** interpret (char*);

#endif