
#ifndef PROJECT_BASE_V1_0_TCB_H
#define PROJECT_BASE_V1_0_TCB_H

#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"
#include "../h/Scheduler.h"
class TCB {
private:
    friend class Riscv;

    static int ID;
    int id=ID++;

    struct Context{
        uint64 ra;
        uint64 sp;
        uint64 sp2;
    };

    uint64 * stack;
    uint64 * stack2;
    Context context;
    bool finished;
    bool blocked;
    bool sleeping;
    bool kernelThread;
    uint64 timeSlice;
    //time_t sleepFor;

    using Body=void(*)(void*);
    void* arg;
    Body body;


    static void threadWrapper();
    static void contextSwitch(Context *oldContext, Context *runningContext);



    TCB(Body b, void* argument);

    void* operator new(size_t size);

public:
    //static methods and pointers.

    static TCB* running;
    static TCB* idleThread;
    static TCB* userThread;
    static TCB* mainThread; 
    static uint64 timeSliceCounter;


    static void dispatch();

    static TCB* createThread(Body body1, void* argument);
    static void idler(void* arg); 
                                  

    //kernel threads
    void kernelize(){kernelThread= true;}
    bool isKernelThread() const{ return kernelThread;}
    //finish
    void finish(){ finished= true;}
    bool isFinished() const{return finished;}

    //block
    void block(){blocked=true;}
    void unblock(){blocked=false;}
    bool isBlocked() const {return blocked;}

    //sleep
    void asleep(){sleeping=true;}
    void awaken(){sleeping=false;}
    bool isSleeping() const{return sleeping;}
    uint64 getTimeSlice() const { return timeSlice; }
    time_t sleepFor;

    TCB* next;
    TCB* nextHash; 

    int getID() const{return id;}

    void operator delete (void* al);

    ~TCB(){
           MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
           memoryAllocator->deallocate(stack);
           memoryAllocator->deallocate(stack2);
    }

};


#endif //PROJECT_BASE_V1_0_TCB_H
