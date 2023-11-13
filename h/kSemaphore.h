#ifndef PROJECT_BASE_V1_0_KSEMAPHORE_H
#define PROJECT_BASE_V1_0_KSEMAPHORE_H

#include "../lib/hw.h"
#include "TCB.h"


class TCB;


class kSemaphore{
private:
    kSemaphore(){
        blockedQueue = nullptr;
        semReturn = -1;
    }

// friend class Riscv;
    static int ID;
    int id=ID++;
    int val;
    void block();
    void deblock();
    TCB* blockedQueue;
    int semReturn;
    bool kernelSemaphore;
    void* operator new(size_t size);
    void* operator new(size_t size, void* adr);
    void operator delete (void* al);
    ~kSemaphore();
public:
    static kmem_cache_t *cache;
  /*  kSemaphore(int value, bool k = false): val(value), semReturn(0) { blockedQueue = nullptr;
        nextHash= nullptr; kernelSemaphore = k;}
  */  int wait(); 
    int signal();

    int getVal() const  {return val;}
    int getID() const {return id;}
    kSemaphore* nextHash;

    static kSemaphore* createSemaphore(int, bool k =false);
    static void _init(void *);
    static void _deinit(void *);
    void init(int, bool);
    void deinit();

};




#endif //PROJECT_BASE_V1_0_KSEMAPHORE_H
