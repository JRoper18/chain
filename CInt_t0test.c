#include <stdio.h>
#include "CInterpreter.c"
int main (int argc, char** argv)
{
    if (argc > 1)
    {
        functionInfo** infos = interpret (argv [1]);
        printf ("modifiers: %s\n", infos [0]->modifiers);
        printf ("name: %s\n", infos [0]->name);
    }
    else
    {
        printf ("Not enough args.");
    }
}