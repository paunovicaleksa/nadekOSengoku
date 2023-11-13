#include "../h/MemoryAllocator.h"
#include "../h/Riscv.h"
#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/kSemaphore.h"
#include "../h/kConsole.h"
#include "../h/syscall_c.h"
#include "../h/HashTable.h"
#include "../h/printing.hpp"


extern void userMain();

void userWrapper(void *){
    userMain();
}

void main(){

    kmem_init((void*)HEAP_START_ADDR, 1<<(FREE_MEM_SIZE-1));
    HashTable::init();
    kConsole::init();

    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);
    //main thread
    TCB *thread=TCB::createThread(nullptr, nullptr);
    thread->kernelize();
    TCB::running=thread;
    TCB::mainThread=thread;
    //console threads
    TCB* output=TCB::createThread(kConsole::consoleWrap, nullptr);
    output->kernelize();
    Scheduler::put(output);
    TCB* input=TCB::createThread(kConsole::inputWrap, nullptr);
    input->kernelize();
    Scheduler::put(input);
    //user and idle threads
    TCB* userthread=TCB::createThread(userWrapper, nullptr);
    //userthread->kernelize();
    Scheduler::put(userthread);
    TCB* idle=TCB::createThread(TCB::idler, nullptr);
    TCB::idleThread=idle;
    TCB::userThread=userthread;
    // (eventually) start user program.

    thread_dispatch();

    output->deinit();
    input->deinit();
    idle->deinit();
    kConsole::deinit();
    kmem_cache_destroy(kSemaphore::cache);
}

