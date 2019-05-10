#include "lib.h" 
Function** functions;

int* final;
void mod (int index);
void mod_async (int index)
{
	Value argsIn[1] = {asInt (index)};
	executeFunction (functions [0], argsIn);
}

Value mod_HELPER_D82KT6KF9 (Value* inputArgs)
{
	mod (inputArgs[0].asInt);
}

void mod(int index)
{
    final [index] = final [index] % 1000;
}
int add (int* array, unsigned int i1, unsigned int i2);
void add_async (int* array, unsigned int i1, unsigned int i2)
{
	Value argsIn[3] = {asPointer (array),asInt (i1),asInt (i2)};
	executeFunction (functions [1], argsIn);
}

Value add_HELPER_D82KT6KF9 (Value* inputArgs)
{
	return asInt (add (inputArgs[0].asInt , inputArgs[1].asPointer , inputArgs[2].asInt));
}

int add(int* array, unsigned int i1, unsigned int i2)
{
    array [i1] = array [i1 % 1000] + array [i2 % 1000];

    return i1;
}
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

	functions [0] = makeFunction (1, mod_HELPER_D82KT6KF9);
	functions [1] = makeFunction (3, add_HELPER_D82KT6KF9);
	waitFor (functions [1], functions [0], 0);
    final = (int*)malloc (100 * sizeof (int));
    printf ("GENERATING RANDOM NUMBERS!\n");
    for (int i = 0; i < 1000; i++)
    {
        final [i] = 1000 - i - 1;
    }
    for (int i = 0; i < 100000; i++)
    {
        add_async (final, i, final [i]);
    }

    for (int i = 0; i < 1000; i++)
    {
        printf ("%d\n", final [i]);
    }
	finish ();
}