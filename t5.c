#include <unistd.h>
#include "pool.h"
#include "functions.h"
#include "iterators.h"
#include "worker.h"

int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

int fibonacci(int n){
	if(n == 0 || n == 1){
		return 1;
	}
	else {
		return fibonacci(n - 1) + fibonacci(n - 2);
	}
}
Value fib(Value* val){
	int n = val[0].asInt;
	return asInt(fibonacci(n));
}
Value print(Value* val){
	printf("%d\n", val[0].asInt);
}

int main() {
	makeWorkers();
	Function* printFunction = makeFunction(1, print);
	Value arg1 = asInt(212);
	executeFunction(printFunction, &arg1);
	finishAllWorkers();
	sleep(10);
	printf("Desired output: Should iterate through first 10 fibonacci numbers in any order. \n");

	return 0;
}