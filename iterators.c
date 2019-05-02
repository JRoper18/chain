//
// Created by jack on 4/30/2019.
//

#include <stddef.h>
#include "iterators.h"
#include "atomic.h"

void foreach_int(Function* func, int arr[], size_t size){
	Notifier* listNotifier = makeNotifier(func, 0);
	for(int i = 0; i<size; i++){
		int next = arr[i];
		notify(listNotifier, asInt(next));
	}
}
