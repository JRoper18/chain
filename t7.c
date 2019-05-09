//
// Created by jack on 5/7/2019.
//

#include <unistd.h>
#include "pool.h"
#include "functions.h"
#include "iterators.h"
#include "worker.h"
int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

Value storeInt(Value* val){
	(*(int*) val[1].asPointer) = val[0].asInt;
}
Value print(Value* val){
	printf("%d\n", val[0].asInt);
}

Function* storeFunction;
Function* fibFunc;

Value fib(Value* args){
	int n = args[0].asInt;
	if(n == 0 || n == 1){
		return asInt(1);
	}
	else {
		volatile int n_1;
		Function* storeToN1 = cloneFunction(storeFunction, 0);
		Function* subFibFunc = cloneFunction(fibFunc, 0);
		setArg(storeToN1, 1, asPointer(&n_1));
		waitFor(subFibFunc, storeToN1, 0);
		Value n1Arg = asInt(n - 1);
		executeFunction(subFibFunc, &n1Arg);
		Value n_2Val = asInt(n-2);
		int n_2 = fib(&n_2Val).asInt;
		localSync();
		return asInt(n_1 + n_2);
	}
}

int main() {
	makeWorkers();
	storeFunction = makeFunction(2, storeInt);
	fibFunc = makeFunction(1, fib);
	Function* printFunction = makeFunction(1, print);

	foreach_int(printFunction, data, 10);
	localSync();
	printf("Desired output: Should iterate through numbers 1-10 in any order. \n");
	waitFor(fibFunc, printFunction, 0);
	foreach_int(fibFunc, data, 10);
	localSync();
	printf("Desired output: Should iterate through numbers 1-10 in any order, with some but hopefully not many duplicates. \n");

	return 0;
}
