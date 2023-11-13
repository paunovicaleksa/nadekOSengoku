#include "../h/MemoryAllocator.h"
MemoryAllocator* MemoryAllocator::memoryAllocator= nullptr;

MemoryAllocator *MemoryAllocator::getinstance() {
    if(!MemoryAllocator::memoryAllocator) {
       memoryAllocator=(MemoryAllocator*)HEAP_START_ADDR;
       size_t alokatorsize=(MEM_BLOCK_SIZE+ sizeof(MemoryAllocator)-1)/MEM_BLOCK_SIZE;
        char* start=(char*)HEAP_START_ADDR + alokatorsize*MEM_BLOCK_SIZE;
        memoryAllocator->fmemhead=(FreeMem*)start;
        memoryAllocator->fmemhead->next=nullptr;
        size_t fmemsize=(MEM_BLOCK_SIZE+ sizeof(FreeMem)-1)/MEM_BLOCK_SIZE;
        size_t sz=(char*)HEAP_END_ADDR-start-fmemsize*MEM_BLOCK_SIZE;
        memoryAllocator->fmemhead->size=sz;

    } 

    return memoryAllocator;
}

void *MemoryAllocator::allocate(size_t sz) {
    FreeMem* cur=fmemhead, *prev= nullptr;

    for(; cur; prev=cur, cur=cur->next){
        if(cur->size>=sz) break;
    }
    if(cur== nullptr) return cur;
    size_t fmemsize=(sizeof (FreeMem)+MEM_BLOCK_SIZE-1)/MEM_BLOCK_SIZE;
    fmemsize=fmemsize*MEM_BLOCK_SIZE; 

    size_t  blknum=(sz+MEM_BLOCK_SIZE-1)/MEM_BLOCK_SIZE;
    size_t blksz=blknum*MEM_BLOCK_SIZE;
    size_t remainingsize=cur->size-blksz;
    if(remainingsize>=fmemsize+MEM_BLOCK_SIZE){ 
        cur->size=blksz;
        size_t offset=fmemsize+blksz;
        FreeMem* noviblok=(FreeMem*)((char*)cur+offset);
        if(prev) prev->next=noviblok;
        else fmemhead=noviblok;
        noviblok->size=remainingsize-fmemsize;
        noviblok->next=cur->next;
    }else{
        if(prev) prev->next=cur->next;
        else fmemhead=cur->next;

    }
    cur->next= nullptr;
    

    return (char*)cur+fmemsize;
}

int MemoryAllocator::deallocate(void *allocatedAt) {
    if(allocatedAt== nullptr) return -1;
    size_t fmemsize=(sizeof (FreeMem)+MEM_BLOCK_SIZE-1)/MEM_BLOCK_SIZE;
    fmemsize=fmemsize*MEM_BLOCK_SIZE;

    FreeMem* noviseg=(FreeMem*)((char*)allocatedAt-fmemsize);

    FreeMem* cur;
    if((char*)allocatedAt<(char*)fmemhead){
        cur = nullptr;
    }else{
        for(cur=fmemhead; cur->next; cur=cur->next){
            if((char*)allocatedAt<(char*)cur->next) break;
        }
    }
    if(cur) noviseg->next=cur->next;
    else noviseg->next=fmemhead;
    if(cur) cur->next=noviseg;
    else fmemhead=noviseg;

    if(noviseg->next && (char*)noviseg+noviseg->size+fmemsize==(char*)noviseg->next){
        noviseg->size+=noviseg->next->size+fmemsize;
        noviseg->next=noviseg->next->next;
    }
    if(cur && (char*)cur+cur->size+fmemsize==(char*)noviseg){
        cur->size+=noviseg->size+fmemsize;
        cur->next=noviseg->next;
    }

    return 0;
}



