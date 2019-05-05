#include <stdio.h>
#include "CInterpreter.h"
#include <string.h> \
/*
 *
 */

int main (int argc, char** argv)
{
    functionInfo** infos = interpret ("../CInt_t0.chain", "../CInt_t0.c");
    int i;
    for (i = 0; infos [i] != NULL; i++)
    {
        infoToString (infos [i]);
    }
}