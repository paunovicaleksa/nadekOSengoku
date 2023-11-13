#ifndef PROJECT_BASE_V1_0_KSEMAPHORE_H
#define PROJECT_BASE_V1_0_KSEMAPHORE_H

#include "../lib/hw.h"
#include "TCB.h"


class TCB;


class kSemaphore{
private:
    static int ID;
    int id=ID++;
    int val;
    void block();
    void deblock(); 
    TCB* blockedQueue;
    int semReturn;
    bool kernelSemaphore;
    void* operator new(size_t size);
public:

    kSemaphore(int value): val(value), semReturn(0) { blockedQueue = nullptr; kernelSemaphore=false; nextHash= nullptr;}
    int wait(); 
    int signal();
    void kernelize(){kernelSemaphore=true;}

    int getVal() const  {return val;}
    int getID() const {return id;}
    kSemaphore* nextHash;

    static kSemaphore* createSemaphore(int);

    void operator delete (void* al);
    ~kSemaphore();
};




#endif //PROJECT_BASE_V1_0_KSEMAPHORE_H
