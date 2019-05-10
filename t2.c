//
// Created by jack on 4/27/2019.
//
#include <unistd.h>
#include <pthread.h>
#include "atomic.h"
int x = 0;
Atomic* atomic;
void dumbIncrement(){
    int toAdd = x;
    sleep(1);
    x = toAdd + 1;
}
void smartIncrement(){
    incrementAtomic(atomic, 1);
}
int main() {
	atomic = makeAtomic(asInt(0));
    //Concurrently look and edit our atomic variable incorrectly, by looking at them and waiting a while to edit them.
    //In this test, only 1 edit goes through even though it's called 10 times.
    pthread_t threads[10];
    for(int i = 0; i<10; i++){
        pthread_create(&threads[i], NULL, dumbIncrement, NULL);
    }
    for(int i = 0; i<10; i++){
        pthread_join(threads[i], NULL);
    }
    printf("Should be 10: %d\n", x);
	//Now, concurrently increment it 10 times by using the delta functions.
    for(int i = 0; i<10; i++){
        pthread_create(&threads[i], NULL, smartIncrement, NULL);
    }
    for(int i = 0; i<10; i++){
        pthread_join(threads[i], NULL);
    }
    printf("Should be 10: %d\n", getAtomic(atomic).asInt);


}