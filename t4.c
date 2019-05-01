#include "pool.h"
#include "functions.h"
#include "iterators.h"

int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

void print(Value* val){
	printf("%d\n", val[0].asInt);
}

int main() {
	initPool();
	foreach_int(print, data, 10);
	printf("Desired output: Should iterate through odd numbers.");
	return 0;
}