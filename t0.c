#include "functions.h"
#include "pool.h"

void fe(){
    printf("fe");
}
void fi(){
    printf("fi");
}
void fo(){
    printf("fo");
}
void fum(){
    printf("fum");
}

int main() {
	initPool();
    Function *feFunc = makeFunction(0, fe);
    Function *fiFunc = makeFunction(0, fi);
    Function *foFunc = makeFunction(0, fo);
    Function *fumFunc = makeFunction(0, fum);
    waitFor(feFunc, fiFunc, -1);
    waitFor(fiFunc, foFunc, -1);
    waitFor(foFunc, fumFunc, -1);
    executeFunction(feFunc);
    printf("\nDesired output: fefifofum");
    return 0;
}
