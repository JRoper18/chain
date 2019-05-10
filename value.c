//
// Created by jack on 4/30/2019.
//

#include "value.h"
#include "stdio.h"
#include "stdlib.h"
void addValueQ(ValueQueue* q, Value v) {
	ValueQueueMem* r = calloc(1, sizeof(ValueQueueMem));
	r->val = v;
	r->next = 0;
	if (q->tail != NULL) {
		q->tail->next = r;
	}
	q->tail = r;

	if (q->head == 0) {
		q->head = r;
	}
}
bool isEmptyQ(ValueQueue* q){
	return q->head == 0;
}
Value removeValueQ(ValueQueue* q) {
	ValueQueueMem* r = q->head;
	if (r != 0) {
		q->head = r->next;
		if (q->tail == r) {
			q->tail = 0;
		}
	}
	else {
		return asPointer(0);
	}
	return r->val;
}
