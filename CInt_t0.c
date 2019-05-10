#include "lib.h" 
Function** functions;

void mod (int* array, int index, int modulo);
void mod_async (int* array, int index, int modulo)
{
	Value argsIn[3] = {asPointer (array),asInt (index),asInt (modulo)};
	executeFunction (functions [0], argsIn);
}

Value mod_HELPER_D82KT6KF9 (Value* inputArgs)
{
	mod (inputArgs[0].asInt , inputArgs[1].asPointer , inputArgs[2].asInt);
}

void mod(int* array, int index, int modulo)
{
    array [index] = array [index] % modulo;
}
void add (int* array, int i1, int i2);
void add_async (int* array, int i1, int i2)
{
	Value argsIn[3] = {asPointer (array),asInt (i1),asInt (i2)};
	executeFunction (functions [1], argsIn);
}

Value add_HELPER_D82KT6KF9 (Value* inputArgs)
{
	add (inputArgs[0].asInt , inputArgs[1].asPointer , inputArgs[2].asInt);
}

void add(int* array, int i1, int i2)
{
    array [i1] = array [i1] + array [i2];
    mod (array, i1, 1000);
}
int* final;
int done;
int main (int argc, char** argv);
void main_async (int argc, char** argv)
{
	Value argsIn[2] = {asInt (argc),asPointer (argv)};
	executeFunction (functions [2], argsIn);
}

Value main_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (main (inputArgs[0].asPointer , inputArgs[1].asInt));
}

int main(int argc, char** argv)
{
	initPool ();
	functions = calloc(100, sizeof(Function*));

	functions [0] = makeFunction (3, mod_HELPER_D82KT6KF9);
	functions [1] = makeFunction (3, add_HELPER_D82KT6KF9);
    int* array = (int*)malloc (100 * sizeof (int));
    printf ("GENERATING RANDOM NUMBERS!\n");
    for (int i = 0; i < 1000; i++)
    {
        array [i] = 1000 - i - 1;
    }
    for (int i = 0; i < 100000; i++)
    {
        add_async (array, i, array [i]);
    }

    for (int i = 0; i < 1000; i++)
    {
        printf ("%d\n", array [i]);
    }
	finish ();
}