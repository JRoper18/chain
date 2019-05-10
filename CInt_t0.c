#include "lib.h" 
Function** functions;

int fib (int n);
void fib_async (int n , void** store)
{
	Function* func = functions[0];
	if(store != NULL){
		func=cloneFunction(func, 1);
		Function* storeFunc=makeStoreFunction(store);
		waitFor(func, storeFunc, 0);
	}
	Value* argsIn = calloc(1, sizeof(Value));
	argsIn[0] = asInt (n);
	executeFunction (func, argsIn);
}

Value fib_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (fib (inputArgs[0].asInt));
}

int fib(int n)
{
    if(n < 2){
        return n;
    }
    return fib(n-1) + fib(n-2);
}
int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int nextData (int i);
void nextData_async (int i , void** store)
{
	Function* func = functions[1];
	if(store != NULL){
		func=cloneFunction(func, 1);
		Function* storeFunc=makeStoreFunction(store);
		waitFor(func, storeFunc, 0);
	}
	Value* argsIn = calloc(1, sizeof(Value));
	argsIn[0] = asInt (i);
	executeFunction (func, argsIn);
}

Value nextData_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (nextData (inputArgs[0].asInt));
}

int nextData(int i)
{
    return data[i];
}
int main ();
void main_async (void** store)
{
	Function* func = functions[2];
	if(store != NULL){
		func=cloneFunction(func, 1);
		Function* storeFunc=makeStoreFunction(store);
		waitFor(func, storeFunc, 0);
	}

	executeFunction (func, NULL);
}

Value main_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (main ());
}

int main()
{
	initPool ();
	functions = calloc(100, sizeof(Function*));

	functions [0] = makeFunction (1, fib_HELPER_D82KT6KF9);
	functions [1] = makeFunction (1, nextData_HELPER_D82KT6KF9);
	waitFor (functions [1], functions [0], 0);
    for(int i = 0; i<10; i++){
        int x = 212;
        nextData_async(i, &x);
        sync();
        printf("%d\n", x);
    }
	finish ();
}