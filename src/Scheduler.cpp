#include "../h/Scheduler.h"
#include "../h/TCB.h"

TCB* Scheduler::thread_head[2] = {nullptr, nullptr};
TCB* Scheduler::thread_tail[2]={nullptr, nullptr};
TCB* Scheduler::sleepingThreads = nullptr;

TCB *Scheduler::get() {
    int select = 0;
    if(thread_head[select] == nullptr) {
        select = 1 - select;
        if(thread_head[select] == nullptr) return nullptr;
    }
    TCB* thread = thread_head[select];
    thread_head[select] = thread_head[select]->next;
    if(thread_head[select] == nullptr) thread_tail[select] = nullptr;
    thread->next= nullptr;
    return thread;
}

void Scheduler::put(TCB *thread) {
    int select = (thread->isKernelThread())? 0 : 1;
    if(thread_head[select] == nullptr){
       thread_head[select] = thread_tail[select] = thread;
    } else{
        thread_tail[select]->next = thread;
        thread_tail[select] = thread;
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
