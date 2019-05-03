#include "pool.h"
#include "atomic.h"


int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int output[10];
int mulBy2(Value* args){
	return args[0].asInt * 2;
}
int add1(Value* args){
	return args[0].asInt + 1;
}
void store(Value* args){
	Atomic* atom = ((Atomic*) args[1].asPointer);
	int loc = incrementAtomic(atom, 1) - 1;
	int val = args[0].asInt;
	output[loc] = val;
}

int main() {
	initPool();
	Function* doubleFunc = makeFunction(1, mulBy2);
	Function* incFunc = makeFunction(1, add1);
	Function* strFunc = makeFunction(2, store);
	waitFor(doubleFunc, incFunc, 0);
	waitFor(incFunc, strFunc, 0);
	Atomic* counter = makeAtomic(asInt(0));
	setArg(strFunc, 1, asPointer(counter));
	for(int i = 0; i<10; i++){
		Value* next = calloc(1, sizeof(Value));
		next[0] = asInt(data[i]);
		executeFunction(doubleFunc, next);
	}
	finish();
	for(int i = 0; i<10; i++){
		printf("At location %d is odd number %d\n", i, output[i]);
	}
	printf("Desired output: Should iterate through odd numbers in some order.\n");
	return 0;
}


