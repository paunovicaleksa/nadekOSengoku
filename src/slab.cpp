#include "../h/slab.hpp"
#include "../h/kCache.hpp"
#include "../h/MemoryAllocator.h"
#include "../h/printing.hpp"
#include "../h/BuddyAllocator.h"
#include "../h/misc.h"

const size_t MAX_SLAB_SIZE = ((size_t)BLOCK_SIZE) << (FREE_MEM_SIZE-7);

size_t kmem_estimate(size_t slab_size, size_t obj_size, bool off_slab){
    size_t base = sizeof (FreeObject);
    if(!off_slab){
        base += sizeof (kSlab);
    }

    size_t num;
    size_t left_over;
    while (true){
        num = slab_size/(obj_size+base);
        if(num == 0) {
            slab_size<<=1;
            continue;
        }
        left_over = slab_size - (num * (obj_size+base));
        if((left_over * 8) <= slab_size) //check this code
            break;
        if(slab_size == MAX_SLAB_SIZE) break; //serious fragmentation, but one object can fit
        slab_size<<=1;
    }

    return slab_size;
}


void kmem_init(void *space, int block_num) {

    //block_num is for buddy.

    kCache::cache_cache = (kCache*)space;
    size_t slab_size = BLOCK_SIZE;
    slab_size = kmem_estimate(slab_size, sizeof (kCache), false);
    kCache::cache_cache->init(false, "cache_cache", sizeof(kCache), slab_size, kCache::_init, kCache::_deinit);

    space = (char*)space + sizeof(kCache);

    space = BuddyAllocator::BuddyInit(space, block_num);

    MemoryAllocator::getinstance(space);

    kCache::slab_cache = (kCache*)(kCache::cache_cache->allocate_object());
    slab_size = BLOCK_SIZE;
    slab_size = kmem_estimate(slab_size, sizeof(kSlab), false);
    kCache::slab_cache->init(false, "slab_cache", sizeof(kSlab), slab_size, kSlab::_init, kSlab::_deinit);

}

kmem_cache_t *kmem_cache_create(const char *name, size_t size, void (*ctor)(void *), void (*dtor)(void *)) {

    kCache* cache = (kCache*)kCache::cache_cache->allocate_object();

    size_t slab_size = (size > BLOCK_SIZE)? ceil_2(size) : BLOCK_SIZE;
    slab_size = kmem_estimate(slab_size, size,  true);
    if(slab_size>MAX_SLAB_SIZE) {
        kCache::cache_cache->free_object(cache);
        return nullptr;
    }
    cache->init(true, name, size, slab_size, ctor, dtor);
    return (kmem_cache_t*)cache;
}

void *kmem_cache_alloc(kmem_cache_t *cachep) {
    return ((kCache*)cachep)->allocate_object();
}

void kmem_cache_free(kmem_cache_t *cachep, void *objp) {
    if(objp == nullptr) return;
    ((kCache*)cachep)->free_object(objp);
}

int kmem_cache_shrink(kmem_cache_t *cachep) {
    return ((kCache*)cachep)->shrink();
}

void kmem_cache_info(kmem_cache_t *cachep) {
    kCache* cache = (kCache*)cachep;
    printString(cache->getName());
    putc('\t');
    putc('\t');
    printInt(cache->get_size());
    putc('\t');
    printInt(cache->get_slabs());
    putc('\t');
    printInt(cache->get_object_num());
    putc('\t');
    printInt(cache->get_percent());
    printString("%\n");
}

void kmem_cache_destroy(kmem_cache_t *cachep) {
    kCache* cache = (kCache*)cachep;
    if(cache == kCache::cache_cache || cache == kCache::slab_cache) return;

    cache->deinit(); //destroy all slabs, DOES NOT DEALOCATE OBJECTS
    kCache::cache_cache->free_object(cache);
}

void *kmalloc(size_t size) {
    size_t ceil = ceil_2(size);
    int log = log2_64(ceil);
    int sz = log - 5;
    if(sz<0) return nullptr;
    if (kCache::size_N[sz] == nullptr) {
        kCache::size_N[sz] = (kCache*)(kCache::cache_cache->allocate_object());
        size_t slab_size = (size > BLOCK_SIZE)? ceil_2(size) : BLOCK_SIZE;
        slab_size = kmem_estimate(slab_size, 1<<(sz+5), true);
        kCache::size_N[sz]->init(true, "size_N", 1<<(sz+5), slab_size, nullptr, nullptr);
    }
    return kCache::size_N[sz]->allocate_object();
}

void kfree(const void *objp) {
    kSlab* owner_slab =((kSlab*)((FreeObject*)((char*)objp - sizeof(FreeObject)))->owner);
    if (owner_slab == nullptr) return;
    kCache* cache = (kCache*)(owner_slab->owner);
    cache->free_object((void*)objp);
}

int kmem_cache_error(kmem_cache_t *cachep) {
    kCache* cache = (kCache*)cachep;
    if(cache == nullptr) return -1;
    int err = cache->get_error();
    printString(cache->getName());
    putc('\t');
    if(err | BAD_ALLOC){
        printString("BAD_ALLOC");
        putc('\t');
    }
    if(err | BAD_FREE){
        printString("BAD_FREE");
        putc('\t');
    }
    putc('\n');
    return err;
}