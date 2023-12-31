#ifndef PROJECT_BASE_V1_0_SCHEDULER_H
#define PROJECT_BASE_V1_0_SCHEDULER_H


class TCB;


class Scheduler {
private:

    static TCB* sleepingThreads;
    static TCB* threadQueue;

public:
    static TCB* get();
    static void put(TCB* thread);

    static void putToSleep(TCB* thread);
    static void wakeUp();
};


#endif //PROJECT_BASE_V1_0_SCHEDULER_H
