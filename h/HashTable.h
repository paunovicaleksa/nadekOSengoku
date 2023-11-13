
#ifndef PROJECT_BASE_V1_1_HASHTABLE_H
#define PROJECT_BASE_V1_1_HASHTABLE_H
#include "TCB.h"
#include "kSemaphore.h"

#define HASH_SIZE 256

class HashTable{
private:
    static TCB** threads;
    static kSemaphore** semaphores;

public:
    static void init();

    static TCB* getThread(int);
    static kSemaphore* getSem(int);

    static void insert(TCB*);
    static void insert(kSemaphore*);

    static void remove(TCB*);
    static void remove(kSemaphore*);

};


#endif //PROJECT_BASE_V1_1_HASHTABLE_H
