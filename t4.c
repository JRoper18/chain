#include "pool.h"
#include "functions.h"
#include "iterators.h"

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
void print(Value* val){
	printf("%d\n", val[0].asInt);
}

int main() {
	initPool();
	Function* printFunction = makeFunction(1, print);
	Function* fibFunction = makeFunction(1, fib);
	foreach_int(printFunction, data, 10);
	localSafeSync();
	printf("Desired output: Should iterate through numbers 1-10 in any order. \n");
	waitFor(fibFunction, printFunction, 0);
	foreach_int(fibFunction, data, 10);
	localSafeSync();
	printf("Desired output: Should iterate through first 10 fibonacci numbers in any order. \n");

	return 0;
}