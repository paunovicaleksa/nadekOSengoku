#include "../h/MemoryAllocator.h"
#include "../h/Riscv.h"
#include "../h/Scheduler.h"
#include "../h/TCB.h"
#include "../h/kSemaphore.h"
#include "../h/kConsole.h"
#include "../h/syscall_c.h"


extern void userMain();

void userWrapper(void *){
    userMain();
}

void main(){
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);
    //main thread
    TCB *thread=TCB::createThread(nullptr, nullptr);
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
    Scheduler::put(userthread);
    TCB* idle=TCB::createThread(TCB::idler, nullptr);
    TCB::idleThread=idle;
    TCB::userThread=userthread;
    // (eventually) start user program.
    thread_dispatch();


    delete output;
    delete input;
    //delete userthread; 
    delete idle;
    kConsole* cnsl=kConsole::getinstance();
    delete cnsl;
}

