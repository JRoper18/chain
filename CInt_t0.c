#include "functions.h"
#include "pool.h"

Function** functions;

int data1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
'a''b''c''d''e''f''g''h''i''j'};
void processData1_async (int i){

	Value temp0 = asInt (i);
	executeFunction (functions [0], &temp0);
}

Value processData1_HELPER_D82KT6KF9 (Value* i)
{
	return asInt (processData1 (i [0].asInt));
}

int processData1(int i)
{
    return data1[i];

void processData2_async (int i){

	Value temp0 = asInt (i);
	executeFunction (functions [1], &temp0);
}

Value processData2_HELPER_D82KT6KF9 (Value* i)
{
	return asChar (processData2 (i [0].asInt));
}

char processData2(int i)
{
    return data2[i];

void print_async (int index, char ch){

	Value temp0 = asInt (index);
	Value temp1 = asChar (ch);
	executeFunction (functions [2], &temp0, &temp1);
}

Value print_HELPER_D82KT6KF9 (Value* index, Value* ch)
{
	print (index [0].asInt ch [1].asChar);
}

void print(int index, char ch)
{
    printf("char at index %d is %c", index, ch);

void main_async (){

	executeFunction (functions [3]);
}

Value main_HELPER_D82KT6KF9 ()
{
	return asInt (main ());
}

int main()
{
	makeWorkers ();
	functions [0] = makeFunction (processData1_HELPER_D82KT6KF9, 1);
	functions [1] = makeFunction (processData2_HELPER_D82KT6KF9, 1);
	functions [2] = makeFunction (print_HELPER_D82KT6KF9, 2);
	waitFor (functions [0], functions [2], 0);
	waitFor (functions [1], functions [2], 1);
    for(int i = 0; i<10; i++){
        processData1_async(i);
        processData2_async(i);
    }
	finishAllWorkers ();
}