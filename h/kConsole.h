
#ifndef PROJECT_BASE_V1_0_KCONSOLE_H
#define PROJECT_BASE_V1_0_KCONSOLE_H
#include "../lib/hw.h"
#include "../h/kSemaphore.h"
#define BUFF_SIZE 1024

class kConsole{
public:
    static void init();
    static void deinit();
    static void consoleWrap(void*);
    static void inputWrap(void*);


    static void putc(char c);
    static void send(void*); 

    static char getc();
    static void load();

    static kSemaphore* ready;
    static kSemaphore* inputready;

private:
    static volatile char toAdd;
    static char *outputBuffer; 
    static char *inputBuffer;
    static int head, tail;


    static  kSemaphore *spaceAvailable, *itemsAvailable;
    static kSemaphore *spaceInput, *itemsInput; 
    static int inputHead, inputTail;

};



#endif //PROJECT_BASE_V1_0_KCONSOLE_H
