#include "lib.h" 
Function** functions;

int data1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
char data2[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
int processData1 (int i);
void processData1_async (int i)
{
	Value argsIn[1] = {asInt (i)};
	executeFunction (functions [0], argsIn);
}

Value processData1_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (processData1 (inputArgs[0].asInt));
}

int processData1(int i)
{
    return data1[i];
}
char processData2 (int i);
void processData2_async (int i)
{
	Value argsIn[1] = {asInt (i)};
	executeFunction (functions [1], argsIn);
}

Value processData2_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asChar (processData2 (inputArgs[0].asInt));
}

char processData2(int i)
{
    return data2[i];
}
void print (int index, char ch);
void print_async (int index, char ch)
{
	Value argsIn[2] = {asInt (index),asChar (ch)};
	executeFunction (functions [2], argsIn);
}

Value print_HELPER_D82KT6KF9 (Value* inputArgs)
{
	print (inputArgs[0].asChar , inputArgs[1].asInt);
}

void print(int index, char ch)
{
    printf("char at index %d is %c", index, ch);
}
int main ();
void main_async ()
{

	executeFunction (functions [3], NULL);
}

Value main_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (main ());
}

int main()
{
	makeWorkers ();
	functions = calloc(100, sizeof(Function*));

	functions [0] = makeFunction (1, processData1_HELPER_D82KT6KF9);
	functions [1] = makeFunction (1, processData2_HELPER_D82KT6KF9);
	functions [2] = makeFunction (2, print_HELPER_D82KT6KF9);
	waitFor (functions [0], functions [2], 0);
	waitFor (functions [1], functions [2], 1);
    for(int i = 0; i<10; i++){
        processData1_async(i);
        processData2_async(i);
    }
	finishAllWorkers ();
}}