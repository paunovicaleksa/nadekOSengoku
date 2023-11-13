#include "../h/kConsole.h"
#include "../h/Riscv.h"
#include "../h/MemoryAllocator.h"
#include "../h/TCB.h"
#include "../h/syscall_c.h"
kConsole* kConsole::cnsl= nullptr;
volatile char kConsole::toAdd='5';

kConsole *kConsole::getinstance() {
    if(cnsl== nullptr){
        cnsl=new kConsole();
    }
    return cnsl;
}

void *kConsole::operator new(size_t sz) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    return memoryAllocator->allocate(sz);
}


void kConsole::operator delete(void *arg) {
    MemoryAllocator* memoryAllocator=MemoryAllocator::getinstance();
    memoryAllocator->deallocate(arg);
}


void kConsole::putc(char c) {
    volatile char d=c;


    cnsl->spaceAvailable->wait();


    cnsl->outputBuffer[cnsl->tail]=d;
    cnsl->tail=(cnsl->tail+1)%BUFF_SIZE;

    cnsl->itemsAvailable->signal();

}

void kConsole::send(void *) {

    while(!TCB::running->isFinished()){ 

        ready->wait();
        while (((*(char *)CONSOLE_STATUS )& CONSOLE_TX_STATUS_BIT)){
            itemsAvailable->wait();
            char *writeRegister=(char*)CONSOLE_TX_DATA;
            *writeRegister=outputBuffer[head];
            head=(head+1)%BUFF_SIZE;
            spaceAvailable->signal(); 
            outCount--;
        }

    }

}

void kConsole::consoleWrap(void *) {
    kConsole* cnsl1=kConsole::getinstance();
    cnsl1->send(nullptr);
}

char kConsole::getc() {
    itemsInput->wait();
    char c=inputBuffer[inputTail];
    inputTail=(inputTail+1)%BUFF_SIZE;
    spaceInput->signal();
    return c;

}

void kConsole::load() {
    while (!TCB::running->isFinished()){
        inputready->wait();
        while (((*(char *)CONSOLE_STATUS )& CONSOLE_RX_STATUS_BIT)){
            spaceInput->wait();
            char *readRegister=(char*)CONSOLE_RX_DATA;
            inputBuffer[inputHead]=*readRegister;
            inputHead=(inputHead+1)%BUFF_SIZE;
            itemsInput->signal();
        }

    }
}

void kConsole::inputWrap(void *) {
    kConsole* cnsl1=kConsole::getinstance();
    cnsl1->load();
}

kConsole::kConsole() {
    spaceAvailable=kSemaphore::createSemaphore(BUFF_SIZE);
    spaceAvailable->kernelize();
    itemsAvailable= kSemaphore::createSemaphore(0);
    itemsAvailable->kernelize();
    ready= kSemaphore::createSemaphore(0);
    ready->kernelize();
    spaceInput=kSemaphore::createSemaphore(BUFF_SIZE);
    spaceInput->kernelize();
    itemsInput=kSemaphore::createSemaphore(0);
    itemsInput->kernelize();
    inputready=kSemaphore::createSemaphore(0);
    inputready->kernelize();
    head=0, tail=0;
    outCount=0;
    inputHead=inputTail=0;

}

kConsole::~kConsole() {
    delete spaceAvailable;
    delete spaceInput;
    delete ready;
    delete inputready;
    delete itemsInput;
}

