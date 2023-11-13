#include "../h/kSemaphore.h"
#include "../h/TCB.h"
#include "../h/syscall_c.h"
#include "../h/HashTable.h"

int kSemaphore::ID=0;

void *kSemaphore::operator new(size_t size) {


    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    return memoryAllocator->allocate(size);
}

void kSemaphore::operator delete(void *al) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    memoryAllocator->deallocate(al);
}

kSemaphore::~kSemaphore() {

    while(blockedQueue){
        TCB* sled=blockedQueue->next;
        Scheduler::put(blockedQueue);
        blockedQueue=sled;

    }
    blockedQueue= nullptr;
    semReturn=-1;

}

int kSemaphore::wait() {
    if(--val<0){
        block();
    }

    return semReturn;
}

int kSemaphore::signal() {
    if(val++<0){
        deblock();
    }
    return semReturn;
}

void kSemaphore::block() {

    TCB::running->block();
    if(blockedQueue== nullptr) blockedQueue=TCB::running;
    else{
        TCB* cur;
        for(cur=blockedQueue; cur->next; cur=cur->next);
        cur->next=TCB::running;
        TCB::running->next= nullptr;
    }

    TCB::timeSliceCounter=0;

    if(TCB::running->isKernelThread()){
        thread_dispatch();
    }else{
        TCB::dispatch();
    }
    /*if(kernelSemaphore){
        thread_dispatch();
    }else{
        TCB::dispatch();
    }*/

}

void kSemaphore::deblock() {
    if(blockedQueue== nullptr) return;
    TCB* thread=blockedQueue;
    blockedQueue=blockedQueue->next;
    thread->next= nullptr;
    thread->unblock();
    Scheduler::put(thread);


}

kSemaphore *kSemaphore::createSemaphore(int init) {
    kSemaphore* sem=new kSemaphore(init);
    HashTable::insert(sem);
    return sem;
}




