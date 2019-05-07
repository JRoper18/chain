//
// Created by jack on 5/4/2019.
//

#include <unistd.h>
#include <pthread.h>
#include <syscall.h>
#include "worker.h"
#include "functions.h"

#define MAX_THREADS 30
TaskQ* makeTaskQ(){
	TaskQ* new = calloc(1, sizeof(TaskQ));
	new->deepestLevel = 0;
	for(int i = 0; i<MAX_LEVEL; i++){
		new->levels[i] = 0;
		new->tasksRemaining[i] = 0;
	}
}
void addLevelTask(TaskQ* q, Task* task, int level){
	Task* head = q->levels[level];
	task->next = 0;
	if(head == 0){
		q->levels[level] = task;
	}
	else {
		while(head->next != 0){
			head = head->next;
		}
		head->next = task;
	}
	q->tasksRemaining[level]++;
	q->deepestLevel = (q->deepestLevel < level) ? level : q->deepestLevel;
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
Task* stealShallowest(TaskQ* q){
	Task* stolen = 0;
	int level = 0;
	while(stolen == 0){
		if(level > 99){
			//Checked every level.
			return 0;
		}
		//Go to the shallowest, farthest task in the other ready queue.
		Task* beforeStolen = q->levels[level];
		if(beforeStolen == 0){ //No tasks on this level.
			level++;
			continue;
		}
		if(beforeStolen->next == 0){
			//Only one task at this level. Take it.
			q->levels[level] = 0; //Remove it.
			stolen = beforeStolen;
			break;
		}
		while(beforeStolen->next->next != 0){
			beforeStolen = beforeStolen->next;
		}
		//The stolen task is the one after beforeStolen
		stolen = beforeStolen->next;
		//Remove the task from the queue by severing it from beforeStolen.
		beforeStolen->next = 0;
	}
	if(level == q->deepestLevel){
		//We removed something from the deepest level.
		int newLevel = q->deepestLevel;
		while(q->levels[newLevel] == 0 && newLevel != 0){
			newLevel--;
		}
		q->deepestLevel = newLevel;
	}
	return stolen;
}



static bool finish;

TaskQ* taskQueues[MAX_THREADS];
int threadLevels[MAX_THREADS];
int threadIds[MAX_THREADS];
volatile bool init = 0;
static int numThreads;
pthread_t threads[MAX_THREADS];
int getThreadId(){
	return syscall(__NR_gettid);
}
int getIndexFromThreadId(int threadId){
	for(int i = 0; i<numThreads; i++){
		if(threadIds[i] == threadId){
			return i;
		}
	}
	return -1;
}

void threadProgram(int index){
	threadIds[index] = getThreadId();
	while(!init){
		//Wait until all the threads are done being created
	}
	while(!finish){
		Task* task;
		if(!areTasks(taskQueues[index])){
			//If there's no tasks for us to do, we STEAL from another random thread.
			TaskQ* otherQ;
			do {
				int otherIndex = index;
				while(otherIndex == index){
					otherIndex = (rand() % numThreads);
				} //Get a DIFFERENT thread to steal from.
				otherQ = taskQueues[otherIndex];
			} while(!areTasks(otherQ));
			task = stealShallowest(otherQ);
			if(task == 0){
				continue; //No tasks here.
			}
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
		//It should decrement the counter on the spawning taskQ.
		int spawnId = task->spawningId;
		taskQueues[spawnId]->tasksRemaining[task->level]--;
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
				newTask->spawningId = index;
				int deepest = taskQueues[index]->deepestLevel;
				newTask->level = deepest;
				addLevelTask(taskQueues[index], newTask, deepest);
			}
			notifier = notifier->next;
		}
	}
	pthread_exit(NULL);
}

void makeWorkers(){
	_THREAD_POOL_ = 0;
	numThreads = sysconf(_SC_NPROCESSORS_ONLN);
	taskQueues[0] = makeTaskQ();
	threads[0] = pthread_self();
	threadIds[0] = getThreadId();
	for(uint64_t i = 1; i<numThreads; i++){
		TaskQ* newQ = makeTaskQ();
		taskQueues[i] = newQ;
		pthread_create(&threads[i], NULL, (void*) threadProgram, (void*) i);
	}
	init = 1;
}

void addTask(Function* func, Value* args){
	//Who's trying to add a task right now?
	int threadId = getThreadId();
	int indexId = getIndexFromThreadId(threadId);
	Task* newTask = malloc(sizeof(Task));
	newTask->func = func;
	newTask->args = args;
	newTask->spawningId = indexId;
	newTask->level = threadLevels[indexId] + 1;
	//Add it to the tail of the queue, next to be done.
	addLevelTask(taskQueues[indexId], newTask, threadLevels[indexId] + 1);
}
void localSync(){
	while(!init){} //Wait until everything is started.
	while(true){
		bool remaining = false;
		int threadId = getThreadId();
		int indexId = getIndexFromThreadId(threadId);
		int levelToSyncBelow = taskQueues[indexId]->deepestLevel;
		for(int j = levelToSyncBelow; j<MAX_LEVEL; j++){
			if(taskQueues[indexId]->tasksRemaining[j] > 0){
				//There's a task remaining.
				remaining = true;
				break;
			}
		}
		if(!remaining){
			break;
		}
	}
}
void finishAllWorkers(){
	while(!init){} //Wait until everything is started.
	while(true){
		bool remaining = false;
		for(int i = 0; i<numThreads; i++){
			bool breakOut = false;
			for(int j = 0; j<MAX_LEVEL; j++){
				if(taskQueues[i]->tasksRemaining[j] > 0){
					//There's a task remaining.
					remaining = true;
					breakOut = true;
					break;
				}
			}
			if(breakOut){
				break;
			}
			//Done.
		}
		if(!remaining){
			break;
		}
	}
}

