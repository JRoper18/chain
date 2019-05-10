#include "functions.h"
#include "CInterpreter.h"
#include "atomic.h"
int main() {
	Atomic* x = makeAtomic(asInt(0));
	for(int i = 0; i<1000000; i++){
		incrementAtomic(x, 1);
	}
}