#include <unistd.h>
#include "pool.h"
#include "functions.h"
#include "iterators.h"
#include "worker.h"

Value print(Value* val){
	printf("%d\n", val[0].asInt);
}

int main() {
	makeWorkers();
	Function* printFunction = makeFunction(1, print);
	Value arg1 = asInt(212);
	executeFunction(printFunction, &arg1);
	localWorkerSync();
	printf("Desired output: Should print 212.\n");

	return 0;
}