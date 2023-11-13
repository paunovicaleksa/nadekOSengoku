
#ifndef PROJECT_BASE_V1_0_KCONSOLE_H
#define PROJECT_BASE_V1_0_KCONSOLE_H
#include "../lib/hw.h"
#include "../h/kSemaphore.h"
#define BUFF_SIZE 1024

class kConsole{
public:
    static kConsole* getinstance();
    static void consoleWrap(void*);
    static void inputWrap(void*);

    void operator delete(void * arg);
    ~kConsole();

    void putc(char c);
    void send(void*); 

    char getc();
    void load();

    kSemaphore* ready;
    kSemaphore* inputready;

private:
    static volatile char toAdd;
    char outputBuffer[BUFF_SIZE]; 
    char inputBuffer[BUFF_SIZE];
    int head, tail;
    int outCount;

    kConsole();


    kSemaphore *spaceAvailable, *itemsAvailable;
    kSemaphore *spaceInput, *itemsInput; 
    int inputHead, inputTail;


    static kConsole* cnsl;


    void* operator new(size_t sz);


};



#endif //PROJECT_BASE_V1_0_KCONSOLE_H
