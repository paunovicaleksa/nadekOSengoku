#include "../h/HashTable.h"

TCB* HashTable::threads[256]={};
kSemaphore* HashTable::semaphores[256]={};

TCB *HashTable::getThread(int id) {
    int readfrom=id%256;
    TCB* ret= nullptr;
    if(threads[readfrom]== nullptr) return nullptr;
    if(threads[readfrom]->getID()==id) ret = threads[readfrom];
    else{
        TCB* cur=threads[readfrom];
        while(cur!= nullptr){
            cur=cur->nextHash;
            if(cur->getID()==id) {
                ret=cur;
                break;
            }
        }
    }
    return ret;
}

void HashTable::insert(TCB *thread) {
    int insertat=thread->getID()%256;

    if(threads[insertat]== nullptr) threads[insertat]=thread;
    else{
        TCB* cur=threads[insertat];
        while(cur->nextHash){cur=cur->nextHash;}
        cur->nextHash=thread;
    }

}

kSemaphore *HashTable::getSem(int id) {
    int readfrom=id%256;
    kSemaphore* ret= nullptr;
    if(semaphores[readfrom]== nullptr) return nullptr;
    if(semaphores[readfrom]->getID()==id) ret = semaphores[readfrom];
    else{
        kSemaphore* cur=semaphores[readfrom];
        kSemaphore* prev= nullptr;
        while(cur!= nullptr){
            prev=cur;
            if(prev->getID()==id) {
                ret=prev;
                break;
            }
            cur=cur->nextHash;
        }
    }
    return ret;
}

void HashTable::insert(kSemaphore * sem) {
    int insertat=sem->getID()%256;

    if(semaphores[insertat]== nullptr) semaphores[insertat]=sem;
    else{
        kSemaphore* cur=semaphores[insertat];
        while(cur->nextHash){cur=cur->nextHash;}
        cur->nextHash=sem;
    }
}

void HashTable::remove(kSemaphore *sem) {
    int entry=sem->getID()%256;
    if(semaphores[entry]==sem){
        kSemaphore* old=semaphores[entry];
        semaphores[entry]=old->nextHash; 
        old->nextHash= nullptr;

    } else{
        kSemaphore* cur=semaphores[entry];
        kSemaphore* prev= nullptr; 
        for(;cur!=sem; prev =cur, cur=cur->nextHash);
        prev->nextHash=cur->nextHash;
        cur->nextHash= nullptr;

    }

}

void HashTable::remove(TCB *thread) {
    int entry=thread->getID()%256;
    
    if(threads[entry]==thread){
        TCB* old=threads[entry];
        threads[entry]=old->nextHash; 
        old->nextHash= nullptr;

    } else{
        TCB* cur=threads[entry];
        TCB* prev= nullptr; 
        for(;cur!=thread; prev =cur, cur=cur->nextHash);
        prev->nextHash=cur->nextHash;
        cur->nextHash= nullptr;

    }


}

