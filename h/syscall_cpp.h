#ifndef PROJECT_BASE_V1_0_SYSCALL_CPP_H
#define PROJECT_BASE_V1_0_SYSCALL_CPP_H
#include "syscall_c.h"

void* operator new (size_t);
void operator delete (void*);



class Thread {
public:
    Thread (void (*body)(void*), void* arg);
    virtual ~Thread ();
    int start ();
    static void dispatch ();
    static int sleep (time_t);
protected:
    Thread ();
    virtual void run () {}
private:
    thread_t myHandle;
    friend void cppWrapper(void*);
};




class Semaphore {
public:
    Semaphore (unsigned init = 1);
    virtual ~Semaphore ();
    int wait ();
    int signal ();
private:
    sem_t myHandle;
};

class PeriodicThread : public Thread {
protected:
    PeriodicThread (time_t period);
    virtual void periodicActivation () {} 
    friend void periodicWrapper(void* arg);
};
class Console {
public:
    static char getc ();
    static void putc (char);
};

#endif //PROJECT_BASE_V1_0_SYSCALL_CPP_H
