//
// Created by jack on 4/27/2019.
//
#include <unistd.h>
#include <pthread.h>
#include "atomic.h"
void dumbIncrement(Atomic* atomic){
    int atomicInt = getAtomic(atomic).asInt;
    Value incremented = asInt(atomicInt + 1);
    sleep(1);
    setAtomic(atomic, incremented);
}
Value incrementVal(Value val){
    return asInt(val.asInt+1);
}
void smartIncrement(Atomic* atomic){
    deltaAtomic(atomic, incrementVal);
}
int main() {
    Atomic* atomic = makeAtomic(asInt(7));
    //Concurrently look and edit our atomic variable incorrectly, by looking at them and waiting a while to edit them.
    //In this test, only 1 edit goes through even though it's called 10 times.
    pthread_t threads[10];
    for(int i = 0; i<10; i++){
        pthread_create(&threads[i], NULL, dumbIncrement, atomic);
    }
    for(int i = 0; i<10; i++){
        pthread_join(threads[i], NULL);
    }
    printf("Should be 8: %d\n", getAtomic(atomic).asInt);
    //Now, concurrently increment it 10 times by using the delta functions.
    for(int i = 0; i<10; i++){
        pthread_create(&threads[i], NULL, smartIncrement, atomic);
    }
    for(int i = 0; i<10; i++){
        pthread_join(threads[i], NULL);
    }
    printf("Should be 18: %d\n", getAtomic(atomic).asInt);


}