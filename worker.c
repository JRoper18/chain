//
// Created by jack on 5/4/2019.
//

#include "worker.h"
#include "functions.h"

void addTask(TaskQ* q, Function* v) {
	FunctionWrapper* r = calloc(1, sizeof(FunctionWrapper));
	r->func = v;
	r->next = 0;
	r->prev = q->tail;
	if (q->tail != 0) {
		q->tail->next = r;
	}
	q->tail = r;

	if (q->head == 0) {
		q->head = r;
	}
}
bool areTasks(TaskQ* q){
	return q->head == 0;
}
Function* removeHeadTask(TaskQ* q) {
	FunctionWrapper* r = q->head;
	if (r != 0) {
		q->head = r->next;
		if (q->tail == r) {
			q->tail = 0;
		}
	}
	return r->func;
}
Function* removeTailTask(TaskQ* q){
	FunctionWrapper* t = q->tail;
	t->prev->next = 0;
	t->prev = 0;
	return t->func;
}

