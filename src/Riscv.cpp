#include "../h/Riscv.h"
#include "../lib/console.h"
#include "../h/MemoryAllocator.h"
#include "../h/TCB.h"
#include "../h/kSemaphore.h"
#include "../h/kConsole.h"
#include "../h/HashTable.h"

void Riscv::usermode() {
    __asm__ volatile("csrw sepc, ra");

   if(!TCB::running->isKernelThread()){
        mc_sstatus(Riscv::SSTATUS_SPP);
    }

    __asm__ volatile("sret");

}


void Riscv::handleSupervisorTrap(){

    uint64 scause=r_scause();
    if(scause == 0x0000000000000008UL || scause == 0x0000000000000009UL){
        uint64 a0;
        __asm__ volatile("ld a0, 10*8(s0)");
        __asm__ volatile("mv %0, a0" : "=r" (a0));
        volatile uint64 sepc=r_sepc()+4;
        volatile uint64 sstatus = r_sstatus();
        switch(a0){
            //razliciti sistemski pozivi, potrudio sam se da budfe sto preglednije
            case 1:{
                //mem alloc
                uint64 size;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1" : "=r" (size));
                MemoryAllocator* mem=MemoryAllocator::getinstance();
                void* ptr=mem->allocate(size);
                __asm__ volatile("mv a0, %0" : :"r" (ptr));
                __asm__ volatile("sd a0, 10*8(s0)");
                break;
            }
            case 2:{
                //mem_free
                void* tofree;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1" : "=r" (tofree));
                MemoryAllocator* mem=MemoryAllocator::getinstance();
                int ret=mem->deallocate(tofree);

                __asm__ volatile("mv a0, %0" : :"r" (ret));
                __asm__ volatile("sd a0, 10*8(s0)");
                break;
            }case 0x11:{
                //thread create
                volatile TCB::Body b;
                void* arg;
                __asm__ volatile("ld a2, 12*8(s0)");
                __asm__ volatile("mv %0, a2": "=r" (b));
                __asm__ volatile("ld a3, 13*8(s0)");
                __asm__ volatile("mv %0, a3":"=r" (arg));
                TCB* tcb=TCB::createThread(b, arg);
                Scheduler::put(tcb);
                int id;
                id=tcb->getID();
           /*     __asm__ volatile("mv a1, %0" :: "r" (tcb));
                __asm__ volatile("sd a1, 11*8(s0)");*/
                __asm__ volatile ("ld a1, 11*8(s0)");
                __asm__ volatile ("sd %0, (a1)"::"r" (id));

                if(tcb== nullptr || tcb->body== nullptr){
                    __asm__ volatile("li a0, -1");
                }else{
                    __asm__ volatile("li a0, 0");
                }

                __asm__ volatile ("sd a0, 10*8(s0)");
                break;
            }
            case 0x12:{
                //thread exit
                TCB::running->finish();
                TCB::timeSliceCounter = 0;
                TCB::dispatch();
                __asm__ volatile("li a0, -1");
                __asm__ volatile("sd a0, 10*8(s0)");
                break;
            }
            case 0x13:{
                //thread dispatch
                TCB::timeSliceCounter = 0;
                TCB::dispatch();
                break;
            }
            case 0x21:{
                //sem open or init idr
                int val;

                __asm__ volatile("ld a2, 12*8(s0)");
                __asm__ volatile("mv %0, a2" :"=r" (val));
                kSemaphore* sem=kSemaphore::createSemaphore(val);
                /*__asm__ volatile("mv a1, %0" :: "r" (sem));
                __asm__ volatile("sd a1, 11*8(s0)");*/
                int id=sem->getID();

                __asm__ volatile ("ld a1, 11*8(s0)"::"r" (id));
                __asm__ volatile ("sd %0, (a1)"::"r" (id));
                __asm__ volatile("mv a0, zero");
                __asm__ volatile("sd a0, 10*8(s0)");

                break;
            }
            case 0x22:{
                //sem close
                int id;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1":"=r" (id));
                kSemaphore* sem=HashTable::getSem(id);
                if(sem== nullptr) break;
                HashTable::remove(sem);

                delete sem;

                __asm__ volatile("mv a0, zero");
                __asm__ volatile("sd a0, 10*8(s0)");

                break;
            }
            case 0x23:{
                //sem wait
                int id;
                int ret=0;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1":"=r" (id));
                kSemaphore* sem=HashTable::getSem(id);
                if(sem== nullptr) break;
                ret= sem->wait();

                if(TCB::running->isBlocked()){
                    ret=-1;
                    TCB::running->blocked= false;
                }
                __asm__ volatile("mv a0, %0"::"r" (ret));
                __asm__ volatile("sd a0, 10*8(s0)");
                break;
            }
            case 0x24:{
                //sem signal
                int id;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1":"=r" (id));
                kSemaphore* sem=HashTable::getSem(id);
                if(sem== nullptr) break;
                sem->signal();
                __asm__ volatile("mv a0, zero");
                __asm__ volatile("sd a0, 10*8(s0)");
                break;
            }
            case 0x31: {
                //thread sleep
                time_t sleepFor;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1":"=r" (sleepFor));
                if(sleepFor>0){
                    TCB::running->sleepFor = sleepFor;
                    Scheduler::putToSleep(TCB::running);
                    TCB::timeSliceCounter = 0;
                    TCB::dispatch();
                }
                __asm__ volatile("mv a0, zero");
                __asm__ volatile("sd a0, 10*8(s0)");
                break;

            }
            case 0x41:{
                //getc
                kConsole* cnsl=kConsole::getinstance();
                char c=cnsl->getc();
                __asm__ volatile("sd %0, 10*8(s0)" :: "r" (c));

                break;
            }
            case 0x42:{
                //putc
                char c;
                __asm__ volatile("ld a1, 11*8(s0)");
                __asm__ volatile("mv %0, a1" : "=r" (c));
                kConsole* cnsl=kConsole::getinstance();
                cnsl->putc(c);

                break;
            }
            case 0x51:{
                //thread_make
                volatile TCB::Body b;
                void* arg;
                __asm__ volatile("ld a2, 12*8(s0)");
                __asm__ volatile("mv %0, a2": "=r" (b));
                __asm__ volatile("ld a3, 13*8(s0)");
                __asm__ volatile("mv %0, a3":"=r" (arg));
                TCB* tcb=TCB::createThread(b, arg);

              /*  __asm__ volatile("mv a1, %0" :: "r" (tcb));
                __asm__ volatile("sd a1, 11*8(s0)");*/
                int id=tcb->getID();
                __asm__ volatile ("ld a1, 11*8(s0)");
                __asm__ volatile ("sd %0, (a1)"::"r" (id));
                if(tcb== nullptr || tcb->body== nullptr){
                    __asm__ volatile("li a0, -1");
                }else{
                    __asm__ volatile("li a0, 0");
                }

                __asm__ volatile ("sd a0, 10*8(s0)");

                break;
            }
            case 0x52:{
                int id;
                __asm__ volatile ("ld a1, 11*8(s0)");
                __asm__ volatile ("mv %0, a1": "=r" (id));
                TCB* thread=HashTable::getThread(id);
                Scheduler::put(thread);
                if(thread == nullptr){
                    __asm__ volatile("li a0, -1");
                }else{
                    __asm__ volatile("li a0, 0");
                }
                __asm__ volatile ("sd a0, 10*8(s0)");
                break;
            }


        }

        w_sstatus(sstatus);
        w_sepc(sepc);
    }
    else if(scause == 0x8000000000000001){
        TCB::timeSliceCounter++;
        Scheduler::wakeUp();
        if (TCB::timeSliceCounter >= TCB::running->getTimeSlice())
        {


            uint64 volatile sepc = r_sepc();
            uint64 volatile sstatus = r_sstatus();

            TCB::timeSliceCounter = 0;
            TCB::dispatch();

            w_sstatus(sstatus);
            w_sepc(sepc);

        }
        mc_sip(SIP_SSIP);

    }else if (scause == 0x8000000000000009UL)
    {

        volatile  int ret=plic_claim();

        if(ret==CONSOLE_IRQ) {
            //char *statusRegister=(char*)CONSOLE_STATUS;
            kConsole* cnsl=kConsole::getinstance();
            if(((*(char*)CONSOLE_STATUS ) & CONSOLE_TX_STATUS_BIT)){
                if(cnsl->ready->getVal()<1) cnsl->ready->signal();

            }
            if(((*(char*)CONSOLE_STATUS ) & CONSOLE_RX_STATUS_BIT)){
                if(cnsl->inputready->getVal()<1) cnsl->inputready->signal();

            }

        }
        plic_complete(ret);



    } else{
        //error scause, za debagovanje
        while (1);
    }

}



