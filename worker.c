//
// Created by jack on 5/4/2019.
//

#include <unistd.h>
#include <pthread.h>
#include "worker.h"
#include "functions.h"

void addLevelTask(TaskQ* q, Task* task, int level){
	Task* head = q->levels[level];
	while(head != 0){
		head = head->next;
	}
	head = task;
}

bool areTasks(TaskQ* q){
	return q->deepestLevel != 0;
}
Task* removeDeepest(TaskQ* q){
	Task* t = q->levels[q->deepestLevel];
	q->levels[q->deepestLevel] = t->next;
	while(q->levels[q->deepestLevel] == 0){
		q->deepestLevel--;
	}
	t->next = 0;
	return t;
}
static bool finish;

TaskQ** taskQueues;
int* threadLevels;
static int numThreads;
pthread_t* threads;
void threadProgram(int index){
	while(!finish){
		Task* task;
		if(!areTasks(taskQueues[index])){
			//If there's no tasks for us to do, we STEAL from another random thread.

			int otherIndex = index;
			while(otherIndex == index){
				otherIndex = (rand() % numThreads);
			} //Get a DIFFERENT thread to steal from.
			TaskQ* otherQ = taskQueues[otherIndex];
			Task* stolen = 0;
			int level = 0;
			while(stolen == 0){
				//Go to the shallowest, farthest task in the other ready queue.
				stolen = otherQ->levels[level];
				if(stolen == 0){
					level++;
					continue;
				}
				while(stolen->next != 0){
					stolen = stolen->next;
				}
			}
			threadLevels[index] = level;
			//Do that task.
			task = stolen;
		}
		else {
			//We have a task! The arguments for it should be in the task object.
			threadLevels[index] = taskQueues[index]->deepestLevel;
			task = removeDeepest(taskQueues[index]);
		}
		Value (*func)(Value*) = task->func->exec;
		Value result;
		if(task->func->numArgs == 0){
			result = func(NULL);
		}
		else {
			result = func(task->args);
		}
		//Now, it needs to notify all the things that are dependent on it.
		Notifier* notifier = task->func->notify;
		while(notifier != 0){
			Value* args = calloc(notifier->listener->numArgs, sizeof(Value));
			if(notifier->index >= 0){ //A negative index signals that we don't pipe output at all.
				addValueQ(notifier->listener->values[notifier->index], result);
			}
			//Check if the function can execute something.
			bool canRun = true;
			for(int i = 0; i<notifier->listener->numArgs; i++){
				ValueQueue* queue = notifier->listener->values[i];
				bool set = notifier->listener->set[i];
				if(set){
					args[i] = (Value) asPointer(queue);
				}
				else {
					if(isEmptyQ(queue)){
						//Can't run, don't have all of our args.
						canRun = false;
						free(args);
						break;
					}
					args[i] = removeValueQ(queue);
				}
			}
			if(canRun){
				//All the things after this run at the same level.
				Task* newTask = calloc(1, sizeof(Task));
				newTask->func = notifier->listener;
				newTask->args = args;
				addLevelTask(taskQueues[index], newTask, taskQueues[index]->deepestLevel);
			}
			notifier = notifier->next;
		}
	}
	pthread_exit(NULL);
}
void makeWorkers(){
	numThreads = sysconf(_SC_NPROCESSORS_ONLN);
	taskQueues = calloc(numThreads, sizeof(TaskQ));
	threads = calloc(numThreads, sizeof(pthread_t));
	threadLevels = calloc(numThreads, sizeof(int));
	for(int i = 0; i<numThreads; i++){
		taskQueues[i] = calloc(1, sizeof(TaskQ));
		pthread_create(&threads[i], NULL, (void*) threadProgram, i);
	}
}

void addTask(Function* func, Value* args){
	//Who's trying to add a task right now?
	int threadId = pthread_self();
	int indexId = threadId - 1;
	Task* newTask = malloc(sizeof(Task));
	newTask->func = func;
	newTask->args = args;
	//Add it to the tail of the queue, next to be done.
	addLevelTask(taskQueues[indexId], newTask, threadLevels[indexId]);
}
void localSync(){
	int threadId = pthread_self();
	int indexId = threadId - 1;
	int currentLevel = taskQueues[indexId]->deepestLevel;
	while(taskQueues[indexId]->deepestLevel >= currentLevel){
	}
	//Done.
}

