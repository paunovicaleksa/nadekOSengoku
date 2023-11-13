#include "../h/TCB.h"
#include "../h/Riscv.h"
#include "../h/syscall_c.h"
#include "../h/HashTable.h"

int TCB::ID=0;
TCB* TCB::running= nullptr;
uint64 TCB::timeSliceCounter=0;
TCB* TCB:: idleThread= nullptr;
TCB* TCB:: userThread= nullptr;
TCB* TCB:: mainThread= nullptr;




void *TCB::operator new(size_t size) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    return memoryAllocator->allocate(size);
}


void TCB::operator delete(void *al) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    memoryAllocator->deallocate(al);

}

TCB *TCB::createThread(Body body1, void* argument) {
    TCB* tcb=new TCB(body1, argument);
    HashTable::insert(tcb);
    return tcb;
}

void TCB::dispatch() {
    TCB* old=running;
    if(old->isFinished()){HashTable::remove(running);}
    if(!old->finished && !old->isBlocked() && !old->isSleeping() && old!=mainThread && old!=idleThread) Scheduler::put(old); 
    running=Scheduler::get();

    
    if(running== nullptr){
        running=idleThread;
    }
    if (userThread && userThread->isFinished()){
        userThread= nullptr;
        Scheduler::put(mainThread);
    }
    if(old->isFinished()) delete old; 
    contextSwitch(&old->context, &running->context);
}


void TCB::threadWrapper() {
    Riscv::usermode();
    running->body(running->arg);
    running->finish();
    thread_exit();

}

void TCB::idler(void *arg) {
    while(true){

        thread_dispatch();
    }
    
}


TCB::TCB(TCB::Body b, void *argument) {

    MemoryAllocator* mem=MemoryAllocator::getinstance();

    if(b!= nullptr){
        stack=(uint64 *)mem->allocate(DEFAULT_STACK_SIZE*(sizeof(uint64)));

    }else{
        stack= nullptr;
    }

    stack2=(uint64 *)mem->allocate(DEFAULT_STACK_SIZE*(sizeof(uint64)));

    context={(uint64 )&threadWrapper, stack!= nullptr?(uint64)&stack[DEFAULT_STACK_SIZE]:0,
             (uint64)&stack2[DEFAULT_STACK_SIZE]};
    body=b;
    finished= false;
    blocked= false;
    next= nullptr;
    nextHash= nullptr;
    arg=argument;
    timeSlice=DEFAULT_TIME_SLICE;
    sleeping=false;
    kernelThread=false;

    if(body== nullptr){
        __asm__ volatile ("csrw sscratch, %0"::"r" (&stack2[DEFAULT_STACK_SIZE]));
    }

}