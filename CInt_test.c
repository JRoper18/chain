#include <stdio.h>
#include "CInterpreter.h"
#include <string.h> \
/*
 *
 */

int main (int argc, char** argv)
{
    functionInfo** infos = interpret ("../demos/fib.spl", "../CInt_t0.c", 1);
    int i;
    for (i = 0; infos [i] != NULL; i++){
        infoToString (infos [i]);
    }
}