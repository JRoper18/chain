//
// Created by jack on 4/30/2019.
//

#include <stddef.h>
#include "iterators.h"
#include "atomic.h"

void foreach_int(void* func, int arr[], size_t size){
	Function* function = makeFunction(1, func); //Function will be a 1-argument func.
	Notifier* listNotifier = makeNotifier(function, 0);
	for(int i = 0; i<size; i++){
		int next = arr[i];
		notify(listNotifier, asInt(next));
	}
}
