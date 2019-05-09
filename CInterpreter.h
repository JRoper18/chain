#ifndef C_INTERPRETER_H
#define C_INTERPRETER_H

#include <stdbool.h>

const static int STACK_SIZE = 100;

typedef struct params
{
    char* modifiers;
    char* type;
    char* name;
    char** functions;
    int numFuncs;
    bool isArray;
} param;

typedef struct functionInfo
{
    char* modifiers;
    char* type;
    char* name;
    param** parameters;
    char* inner;
    int numParams;
} functionInfo;

typedef struct stack
{
    int height;
    char* vals;
} stack;

extern functionInfo** interpret (char*, char*, bool safeMode);
extern stack* newStack ();
extern void push (stack*, char);
extern char pop (stack*);
extern char* infoToString (functionInfo*);

#endif