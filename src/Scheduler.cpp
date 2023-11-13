#include "../h/Scheduler.h"
#include "../h/TCB.h"

TCB* Scheduler::threadQueue= nullptr;
TCB* Scheduler::sleepingThreads= nullptr;

TCB *Scheduler::get() {
    if(threadQueue== nullptr) return nullptr;
    TCB* thread=threadQueue;
    threadQueue=threadQueue->next;
    thread->next= nullptr;
    return thread;
}

void Scheduler::put(TCB *thread) {

    if(threadQueue==nullptr) threadQueue=thread;
    else{
        TCB* cur;
        for(cur=threadQueue; cur->next; cur=cur->next);
        cur->next=thread;
    }
    thread->next= nullptr;


}

void Scheduler::wakeUp() {
    if(sleepingThreads)sleepingThreads->sleepFor--;
    while(sleepingThreads){

        TCB* sled=sleepingThreads->next;
        if(sleepingThreads->sleepFor==0){
            sleepingThreads->awaken();
            Scheduler::put(sleepingThreads);
        }else{
            break;
        }
        sleepingThreads=sled;

    }
}

void Scheduler::putToSleep(TCB *thread) {

    thread->asleep();
    if(sleepingThreads== nullptr) sleepingThreads=thread;
    else{
        time_t sum=0;
        TCB* cur=sleepingThreads, *prev= nullptr;
        for(;cur; prev=cur, cur=cur->next){
            sum+=cur->sleepFor;
            if(thread->sleepFor<=sum) break;
        }

        if(!prev){
            thread->next=cur;
            sleepingThreads=thread;
            cur->sleepFor-=thread->sleepFor;

        }else{
            thread->next=cur;
            prev->next=thread;
            if(cur) sum-=cur->sleepFor;
            thread->sleepFor=thread->sleepFor-sum;
            if(cur) cur->sleepFor-=thread->sleepFor;
        }

    }

}
