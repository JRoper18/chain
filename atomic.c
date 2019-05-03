//
// Created by jack on 4/30/2019.
//

#include "value.h"
#include "atomic.h"

/**
 * Executes the current function on the value in the atomic holder.
 * @param atomic The atomic to edit.
 * @param func The function (which should take in a Value) to be executed on the value, which should return the new value.
 * @return What the atomic value was changed to.
 */
Value deltaAtomic(Atomic* atomic, Value func(Value)){
	pthread_mutex_lock(&(atomic->lock));
	Value newVal = func(atomic->val);
	atomic->val = newVal;
	pthread_mutex_unlock(&(atomic->lock));
	return newVal;
}
int incrementAtomic(Atomic* atomic, int amount){
	pthread_mutex_lock(&(atomic->lock));
	int val = atomic->val.asInt + amount;
	atomic->val = asInt(val);
	pthread_mutex_unlock(&(atomic->lock));
	return val;
}
void setAtomic(Atomic* atomic, Value val){
	pthread_mutex_lock(&(atomic->lock));
	atomic->val = val;
	pthread_mutex_unlock(&(atomic->lock));
}
Value getAtomic(Atomic* atomic){
	Value ret;
	pthread_mutex_lock(&(atomic->lock));
	ret = atomic->val;
	pthread_mutex_unlock(&(atomic->lock));
	return ret;
}
Atomic* makeAtomic(Value val){
	Atomic* new = malloc(sizeof(Atomic));
	new->val = val;
	if (pthread_mutex_init(&(new->lock), NULL) != 0)
	{
		printf("\n mutex init failed\n");
		exit(0);
	}
	return new;
}
