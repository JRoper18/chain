#ifndef C_INTERPRETER_H
#define C_INTERPRETER_H

#include <stdbool.h>

typedef struct params
{
    char* modifiers;
    char* name;
    char** functions;
    int numFuncs;
} param;

typedef struct functionInfo
{
    char* modifiers;
    char* name;
    param** parameters;
    int numParams;
} functionInfo;

extern char* findLastWord (char*);
extern functionInfo** interpret (char*);

#endif