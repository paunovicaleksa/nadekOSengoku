#include "../h/BuddyAllocator.h"
#include "../h/misc.h"
void* BuddyAllocator::mem = nullptr;
Sz_info BuddyAllocator::sz_info[FREE_MEM_SIZE];

#define FIND_BLOCK_SIZE(k) ((1UL<<(k))*BLOCK_SIZE)
size_t find_block_size(size_t k){
    size_t ret = 1L << k;
    return ret * BLOCK_SIZE;
}

void *BuddyAllocator::BuddyInit(void *start, int blocknum) {
    mem = start;
    //init alloc, split arrays, and list heads
    for(int k = 0; k < FREE_MEM_SIZE; k++){
        sz_info[k].head = nullptr;
        memset(sz_info[k].alloc, 0, MAX_BLOCKS);
        if(k > 0)
            memset(sz_info[k].split, 0, MAX_BLOCKS);
    }
    //point to beginning of free space
    sz_info[FREE_MEM_SIZE - 1].head = (FreeBlock*)start;
    sz_info[FREE_MEM_SIZE - 1].head->next = nullptr;
    return (char*)start + blocknum * BLOCK_SIZE;
}


void *BuddyAllocator::alloc(size_t sz) {
    if(sz == 0) return nullptr;
    int k = log2_64(ceil_2(sz)/BLOCK_SIZE);
    if(k > FREE_MEM_SIZE) return nullptr;
    if(k < 0) k = 0;
    return _alloc(k);
}

void *BuddyAllocator::_alloc(int i){
    int k;
    for(k = i; k < FREE_MEM_SIZE; k++){
        if(sz_info[k].head != nullptr) break;
    }
    if(k == FREE_MEM_SIZE) return nullptr; //no free mem
    void *p = list_pop(k);

    split(i, k, p); //split if necessary

    return p;
}

void BuddyAllocator::split(int i, int k, void* p){
    bit_set(sz_info[k].alloc, block_index(k, (char*)p));
    for(; k > i; k--){
        char *q = (char*)p + find_block_size(k-1);
        bit_set(sz_info[k].split, block_index(k, (char*)p));
        bit_set(sz_info[k-1].alloc, block_index(k-1, (char*)p));
        list_push(k-1,q);
    }

}

void BuddyAllocator::free(void* o, size_t sz){
    int i = log2_64(ceil_2(sz)/BLOCK_SIZE);
    if(i < 0) i = 0;
    _free(o, i);
}
void BuddyAllocator::_free(void* o, int i){
    void* q;
    int k;
    bit_clear(sz_info[i].alloc, block_index(i, (char*)o)); //if largest size was allocated, clear it anyway
    for(k = i; k < FREE_MEM_SIZE - 1; k++){
        int object_i = block_index(k, (char*)o);
        int buddy_i = (object_i % 2 == 0)? object_i+1 : object_i-1; //is oi left or right

        bit_clear(sz_info[k].alloc, object_i); //clear alloc bit

        if(is_bit_set(sz_info[k].alloc, buddy_i)) break;

        q = addr(k, buddy_i);

        if(list_remove((FreeBlock*)q, k)<0)
            break;

        if(buddy_i % 2 == 0) o = q;

        bit_clear(sz_info[k+1].split, block_index(k+1, (char*)o));
    }
    list_push(k, o);
}


void BuddyAllocator::list_push(int k, void *block) {
    FreeBlock* blk = (FreeBlock*) block;
    blk->next = sz_info[k].head;
    sz_info[k].head = blk;
}

void* BuddyAllocator::list_pop(int k){
    FreeBlock* blk = sz_info[k].head;
    sz_info[k].head = sz_info[k].head->next;
    return blk;
}
int BuddyAllocator::list_remove(FreeBlock* q, int k){
    //test this code lol.
    FreeBlock* temp = sz_info[k].head;
    FreeBlock* prev = nullptr;
    while(temp){
        if(temp == q){

            if(temp == sz_info[k].head)
                sz_info[k].head = sz_info[k].head->next;

            if(prev) prev->next = temp->next;

            temp->next = nullptr;
            return 0;
        }
        temp = temp->next;
        prev = temp;
    }
    return -1;
}

void *BuddyAllocator::addr(int k, int bi){
    size_t n = bi * FIND_BLOCK_SIZE(k);
    return (char*)mem + n;
}
int BuddyAllocator::block_index(int k, char* p){
    size_t n = p - (char*)mem;
    return n/FIND_BLOCK_SIZE(k);
}

void BuddyAllocator::bit_set(char *arr, int index){
    char b = arr[index/8];
    char m = (1 << (index % 8));
    arr[index/8] = (b | m);
}

void BuddyAllocator::bit_clear(char* arr, int index){
    char b = arr[index/8];
    char m = (1 << (index % 8));
    arr[index/8] = (b & ~m);
}
bool BuddyAllocator::is_bit_set(char *arr, int index){
    char b = arr[index/8];
    char m = (1 << (index % 8));
    return (b & m) == m;
}
