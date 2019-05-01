#include <sys/wait.h>
#include <unistd.h>
#include "functions.h"
#include "pool.h"

Value waitForANumber(){
    Value numPtr = asInt(1);
    printf("Making a number. Please be patient.\n");
    sleep(1);
    printf("Here's your number: %d.\n", 1);
    return numPtr;
}
void wantsANumber(Value* number){
    printf("Got a number: %d\n", number[0].asInt);
}
int main() {
	initPool();
    Function* numberGiver = makeFunction(0, waitForANumber);
    Function* wantsNumber = makeFunction(1, wantsANumber);
    waitFor(numberGiver, wantsNumber, 0);
    executeFunction(numberGiver);
    printf("\nDesired output: 1");
    return 0;
}
