#ifndef PROJECT_BASE_V1_0_MEMORYALLOCATOR_H
#define PROJECT_BASE_V1_0_MEMORYALLOCATOR_H
#include "../lib/console.h"
#include "../lib/hw.h"
struct FreeMem{
    size_t size;
    FreeMem* next;
};

class MemoryAllocator{
private:
    MemoryAllocator()=default;
    FreeMem* fmemhead;
    static MemoryAllocator* memoryAllocator;
protected:
    void init(void *start);

public:
    static MemoryAllocator* getinstance(void *adr = nullptr);

    void* allocate(size_t sz);
    int deallocate(void* allocatedAt);
};





#endif //PROJECT_BASE_V1_0_MEMORYALLOCATOR_H
