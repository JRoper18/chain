#include <sys/wait.h>
#include <unistd.h>
#include "functions.h"

Value* waitForANumber(){
    Value* numPtr = malloc(sizeof(Value));
    *numPtr = asInt(1);
    printf("Making a number. Please be patient.\n");
    sleep(1);
    printf("Here's your number.\n");
    return numPtr;
}
void wantsANumber(Value** number){
    printf("Got a number: %d\n", number[0]->asInt);
}
int main() {
    Function* numberGiver = makeFunction(0, waitForANumber);
    Function* wantsNumber = makeFunction(1, wantsANumber);
    waitFor(numberGiver, wantsNumber, 0);
    executeFunction(numberGiver);
    printf("\nDesired output: 1");
    return 0;
}
