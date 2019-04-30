#include <stdio.h>
#include "CInterpreter.h"
#include <string.h>
int main (int argc, char** argv)
{
        functionInfo** infos = interpret ("../CInt_t0.chain");
        printf ("modifiers: %s\n", infos[0]->modifiers);
        printf ("name: %s\n", infos [0]->name);
}