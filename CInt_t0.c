#include <stdio.h>
#include "CInterpreter.h"
#include <string.h>
int main (int argc, char** argv)
{
    functionInfo** infos = interpret ("../CInt_t0.chain");
    printf ("name: %s\nmodifiers: %s\n", infos [0]->name, infos[0]->modifiers);
}