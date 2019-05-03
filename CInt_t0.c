#include <stdio.h>
#include "CInterpreter.h"
#include <string.h>
int main (int argc, char** argv)
{
    functionInfo** infos = interpret ("../CInt_t0.chain");
    printf ("%s\n", infoToString (infos [0]));
}