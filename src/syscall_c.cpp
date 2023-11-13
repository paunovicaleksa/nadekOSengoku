#include "../h/syscall_c.h"
#include "../h/TCB.h"


uint64 apiWrapper(uint64 arg0, uint64 arg1=0, uint64 arg2=0, uint64 arg3=0, uint64 arg4=0){
    uint64 ret=-1;

    __asm__ volatile( "ecall");
    __asm__ volatile("mv %0, a0":"=r"(ret));
    return ret;
}


void* mem_alloc (size_t size){

    void* petar=(void *) apiWrapper(0x1, size);

    return petar;
}
int mem_free (void* oslobodi){
    int ret=0;

    ret= apiWrapper(0x2, (uint64 )oslobodi);

    return ret;
}

void thread_dispatch() {
    apiWrapper(0x13);
}

int thread_exit() {
    int ret=0;

    ret= apiWrapper(0x12);
    return ret;

}
int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) {
    int ret=-1;

    ret= apiWrapper(0x11, (uint64)handle, (uint64)start_routine, (uint64)arg);

    return ret;
}
//0x21
int sem_open(sem_t *handle, unsigned int init) {
    int ret=-1;

    ret= apiWrapper(0x21, (uint64)handle, init);
    return ret;
}

//0x22
int sem_close(sem_t handle) {
    int ret=-1;

    ret= apiWrapper(0x22, (uint64 )handle);
    return ret;
}
int sem_wait(sem_t id) {

    int ret=-1;

    ret= apiWrapper(0x23, (uint64)id);
    return ret;

}
int sem_signal(sem_t id) {

    int ret=-1;

    ret= apiWrapper(0x24, (uint64)id);
    return ret;

}

int time_sleep(time_t time) {
    int ret=-1;

    ret= apiWrapper(0x31, time);
    return ret;
}

//0x41
char getc (){
    int ret=EOF;

    ret= apiWrapper(0x41);
    return ret;
}

//0x42
void putc(char c) {
    apiWrapper(0x42, c);

}

int thread_make(thread_t *handle, void (*start_routine)(void *), void *arg) {
    int ret=-1;

    ret= apiWrapper(0x51, (uint64)handle, (uint64)start_routine, (uint64)arg);
    return ret;
}

int thread_start(thread_t handle) {
    int ret=-1;

    ret= apiWrapper(0x52, (uint64)handle);
    return ret;
}
