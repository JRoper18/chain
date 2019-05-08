//
// Created by jack on 5/7/2019.
//

#include <unistd.h>
#include "pool.h"
#include "functions.h"
#include "iterators.h"
#include "worker.h"

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
	Value arg1 = asInt(2);
	waitFor(fibFunc, printFunction, 0);
	executeFunction(fibFunc, &arg1);
	finishAllWorkers();
	printf("Desired output: Should print the 2nd fibonacci number 2 and halt.\n");

	return 0;
}