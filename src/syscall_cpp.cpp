#include "../h/syscall_c.h"
#include "../h/syscall_cpp.h"

void cppWrapper(void *arg){
    Thread* thread=(Thread*)arg;
    thread->run();
}

struct periodicData{
    time_t period;
    PeriodicThread* thread;
    periodicData(PeriodicThread* t, time_t p):period(p), thread(t){}
};

void periodicWrapper(void* arg){
     periodicData* data=(periodicData*)arg;
     while(1){
         data->thread->periodicActivation();
         Thread::sleep(data->period);
     }
    delete data;
}

void operator delete(void * arg) {
    mem_free(arg);
}

void *operator new(size_t sz) {
     return mem_alloc(sz);
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}

Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

Thread::Thread(void (*body)(void *), void *arg) {
    thread_make(&myHandle, body, arg);

}

Thread::~Thread() {

}

int Thread::start() {
    return thread_start(myHandle);
}

void Thread::dispatch() {
    thread_dispatch();

}

int Thread::sleep(time_t t) {
    return time_sleep(t);
}

Thread::Thread() {
    thread_make(&myHandle, cppWrapper, this);

}

PeriodicThread::PeriodicThread(time_t period):Thread(periodicWrapper, new periodicData(this, period)){} 

