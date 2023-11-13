#include "../h/kConsole.h"
#include "../h/Riscv.h"


volatile char kConsole::toAdd='5';
kSemaphore* kConsole::itemsInput = nullptr;
kSemaphore* kConsole::spaceInput = nullptr;
kSemaphore* kConsole::spaceAvailable = nullptr;
kSemaphore* kConsole::inputready = nullptr;
kSemaphore* kConsole::itemsAvailable = nullptr;
kSemaphore* kConsole::ready = nullptr;
char* kConsole::outputBuffer = nullptr;
char* kConsole::inputBuffer = nullptr;
int kConsole::head = 0;
int kConsole::tail = 0;
int kConsole::inputHead = 0;
int kConsole::inputTail = 0;


void kConsole::putc(char c) {
    volatile char d=c;

    spaceAvailable->wait();

    outputBuffer[tail]=d;
    tail=(tail+1)%BUFF_SIZE;

    itemsAvailable->signal();
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
        }
    }
}

void kConsole::consoleWrap(void *) {
    send(nullptr);
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
    load();
}

void kConsole:: init(){
    spaceAvailable=kSemaphore::createSemaphore(BUFF_SIZE, true);
    //spaceAvailable->kernelize();
    itemsAvailable= kSemaphore::createSemaphore(0, true);
    //itemsAvailable->kernelize();
    ready= kSemaphore::createSemaphore(0, true);
    //ready->kernelize();
    spaceInput=kSemaphore::createSemaphore(BUFF_SIZE, true);
    //spaceInput->kernelize();
    itemsInput=kSemaphore::createSemaphore(0, true);
    //itemsInput->kernelize();
    inputready=kSemaphore::createSemaphore(0, true);
    //inputready->kernelize();
    head=0, tail=0;
    inputHead=inputTail=0;
    inputBuffer = (char*) kmalloc(BUFF_SIZE * sizeof (char));
    outputBuffer = (char*) kmalloc(BUFF_SIZE * sizeof (char));
}

void kConsole::deinit(){
    spaceAvailable->deinit();
    spaceInput->deinit();
    ready->deinit();
    inputready->deinit();
    itemsInput->deinit();
    kfree(inputBuffer);
    kfree(outputBuffer);
}


