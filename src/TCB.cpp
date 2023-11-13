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
kmem_cache_t* TCB::cache = nullptr;





TCB *TCB::createThread(Body body1, void* argument) {
    if(cache == nullptr) cache = kmem_cache_create("TCB_cache", sizeof(TCB), TCB::_init, TCB::_deinit);

  
    TCB* tcb = (TCB*) kmem_cache_alloc(cache);
    if(tcb == nullptr) return nullptr;
    void *ret = (TCB*)tcb->init(body1, argument);
    if(ret == nullptr) {
        tcb->deinit();
        kmem_cache_free(cache,tcb);
        return nullptr;
    }
    HashTable::insert(tcb);
    return tcb;
}

void TCB::dispatch() {
    TCB* old=running;
    if(old->isFinished()){HashTable::remove(running);}
    if(!old->finished && !old->isBlocked() && !old->isSleeping() && old!=mainThread && old!=idleThread) Scheduler::put(old); 
    running=Scheduler::get();

    
    if(running == nullptr){
        running=idleThread;
    }
    if (userThread && userThread->isFinished()){
        userThread= nullptr;
        Scheduler::put(mainThread);
    }
    if(old->isFinished()){
        old->deinit(); 
        kmem_cache_free(cache, old);
    }
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


void *TCB::operator new(size_t size, void *adr) {
    return adr;
}

void* TCB::init(TCB::Body b, void *argument) {


    if(b!= nullptr){
        MemoryAllocator* memoryAllocator = MemoryAllocator::getinstance();
        stack=(uint64 *)memoryAllocator->allocate(DEFAULT_STACK_SIZE*(sizeof(uint64)));
       if(stack == nullptr) return nullptr;
    }else{
        stack = nullptr;
    }
    stack2 = (uint64 *)kmalloc(DEFAULT_STACK_SIZE*(sizeof(uint64)));
    if(stack2 == nullptr) return nullptr;
    context={(uint64 )&threadWrapper, stack!= nullptr?(uint64)&stack[DEFAULT_STACK_SIZE]:0,
             (uint64)&stack2[DEFAULT_STACK_SIZE]};
    body = b;
    finished = false;
    blocked = false;
    next = nullptr;
    nextHash = nullptr;
    arg = argument;
    timeSlice = DEFAULT_TIME_SLICE;
    sleeping = false;
    kernelThread = false;

    if(body == nullptr){
        __asm__ volatile ("csrw sscratch, %0"::"r" (&stack2[DEFAULT_STACK_SIZE]));
    }
    return this;
}

void TCB::_init(void *adr) {
    new(adr) TCB();
}

void TCB::_deinit(void *arg) {
    ((TCB*)arg)->~TCB();
}

void TCB::deinit() {
    if(stack) {
        MemoryAllocator* memoryAllocator = MemoryAllocator::getinstance();
        memoryAllocator->deallocate(stack);

    }
    if(stack2)
    {
        kfree(stack2);
    }
    nextHash = nullptr;
    body = nullptr;
    stack = stack2 = nullptr;
    kernelThread = false;
    next = nullptr;
    arg = nullptr;
    context = {0, 0};
}

void TCB::kernel_dispatch() {
    if(TCB::running->isKernelThread()) thread_dispatch();
    else {
        Riscv::pushRegisters();
        TCB::dispatch();
        Riscv::popRegisters();
    }
}
