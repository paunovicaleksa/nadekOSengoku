#include "../h/kSemaphore.h"
#include "../h/Riscv.h"
#include "../h/syscall_c.h"
#include "../h/HashTable.h"

int kSemaphore::ID=0;
kmem_cache_t* kSemaphore::cache = nullptr;

void *kSemaphore::operator new(size_t size) {


    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    return memoryAllocator->allocate(size);
}

void kSemaphore::operator delete(void *al) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    memoryAllocator->deallocate(al);
}

kSemaphore::~kSemaphore() {

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

   if(kernelSemaphore){
        TCB::kernel_dispatch();
        //thread_dispatch();
    }else{
        TCB::dispatch();
    }

}

void kSemaphore::deblock() {
    if(blockedQueue== nullptr) return;
    TCB* thread=blockedQueue;
    blockedQueue=blockedQueue->next;
    thread->next= nullptr;
    thread->unblock();
    Scheduler::put(thread);
}

kSemaphore *kSemaphore::createSemaphore(int init, bool k) {
    if(cache == nullptr) cache = kmem_cache_create("sem_cache", sizeof (kSemaphore), _init, _deinit);

    kSemaphore* sem=(kSemaphore*) kmem_cache_alloc(cache);
    if(sem == nullptr) return nullptr;
    sem->init(init, k);
    HashTable::insert(sem);
    return sem;
}

void kSemaphore::init(int v, bool k) {
    val = v;
    kernelSemaphore = k;
    blockedQueue = nullptr;
    semReturn = 0;
    nextHash = nullptr;
}
void kSemaphore::deinit() {
    val = 0;
    while(blockedQueue){
        TCB* next_thread = blockedQueue->next;
        Scheduler::put(blockedQueue);
        blockedQueue = next_thread;
    }
    nextHash = nullptr;
    blockedQueue = nullptr;
    semReturn = -1;
}

void *kSemaphore::operator new(size_t size, void *adr) {
    return adr;
}

void kSemaphore::_init(void *adr) {
    new(adr) kSemaphore();
}

void kSemaphore::_deinit(void *adr) {
    ((kSemaphore*)adr)->~kSemaphore();
}






