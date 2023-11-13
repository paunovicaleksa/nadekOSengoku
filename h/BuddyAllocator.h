#ifndef NADEKOSENGOKU_BUDDYALLOCATOR_H
#define NADEKOSENGOKU_BUDDYALLOCATOR_H
#include "../lib/hw.h"
#include "../h/slab.hpp"


#define MAX_BLOCKS ((1<<FREE_MEM_SIZE)/8)
struct FreeBlock{
    FreeBlock* next;
};

struct Sz_info{
    FreeBlock* head;
    char alloc[MAX_BLOCKS]; //bitmap
    char split[MAX_BLOCKS];
};

class BuddyAllocator {
public:
    static void* BuddyInit(void* start, int blocknum);
    static void free(void *o, size_t sz);
    static void* alloc(size_t sz);
protected:
    static void list_push(int, void*);
    static void *list_pop(int k);
    static void *_alloc(int k);
    static void _free(void *o, int i);
    static void split(int i, int k, void* p);
    static int list_remove(FreeBlock *q, int k);

private:
    static void* mem;
    static Sz_info sz_info[FREE_MEM_SIZE];

    //bit-array helper methods
    static void *addr(int k, int bi);
    static int block_index(int k, char *p);
    static void bit_set(char *arr, int index);
    static void bit_clear(char *arr, int index);
    static bool is_bit_set(char *arr, int index);

};


#endif //NADEKOSENGOKU_BUDDYALLOCATOR_H
