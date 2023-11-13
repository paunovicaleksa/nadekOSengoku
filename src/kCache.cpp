#include "../h/kCache.hpp"
#include "../h/MemoryAllocator.h"
#include "../h/BuddyAllocator.h"

kCache* kCache::head= nullptr;
kCache* kCache::size_N[]={nullptr};
kCache* kCache::cache_cache = nullptr;
kCache* kCache::slab_cache = nullptr;

#define ALLOCATE 0
#define FREE 1



void *kCache::allocate_object() {
    //get free object slot from slab
    kSlab* free_slab = find_free_slab(used_slabs);
    if(free_slab == nullptr) free_slab = find_free_slab(free_slabs);
    if(free_slab == nullptr) free_slab =(kSlab*) allocate_slab();
    if(free_slab == nullptr) {
        error |= BAD_ALLOC;
        return nullptr;
    }

    void *free_slot = free_slab->allocate();  //check if used_slab should go to full_slabs
    if(free_slot == nullptr) {
        error |= BAD_ALLOC;
        return nullptr;
    }
    reposition(free_slab, ALLOCATE);

    return free_slot;
}

int kCache::free_object(void *obj) {
    //check which slab its on, turn slab from full -> used, or if needed used -> free
    kSlab* target = nullptr;
    target = find_slab(used_slabs, obj);
    if(target == nullptr) target = find_slab(full_slabs, obj);
    if (target == nullptr) {
        error |= BAD_FREE;
        return -1;
    }

    target->free(obj);

    reposition(target, FREE);

    return 0;
}

//finds slab where the object is to be allocated
kSlab *kCache::find_free_slab(kSlab *head) {
    if(head == nullptr) return nullptr;
    return head;
}

//allocates one slab and adds it to the free_slabs list
void *kCache::allocate_slab() {
    //something like this?

    void* adr =BuddyAllocator::alloc(slab_size);
    if(adr == nullptr) return nullptr;
    if(to_shrink == 1) to_shrink = 2;
    kSlab *slab;
    size_t use_size = slab_size;
    if(off_slab){
        slab = (kSlab*)(slab_cache->allocate_object());
        if(slab == nullptr) return nullptr;
    }
    else {
        slab = (kSlab*)adr;
        kSlab::_init(adr);
        adr = (char*)adr + sizeof(kSlab);
        use_size -= sizeof(kSlab);
    }


    slab->init(adr, object_size, use_size, free_slabs, cnstor, dstor, this);
    free_slabs = slab;
    slab->list_head = &free_slabs;
    num++;
    return slab;
}

//repositions slab to a different list, if needed
void kCache::reposition(kSlab *slab, int flag) {


    if(ALLOCATE == flag && (slab->free_object_count >=1 && slab->free_object_count < slab->max_object_count -1)) return;
    if(FREE == flag && (slab->free_object_count > 1 && slab->free_object_count <= slab->max_object_count - 1)) return;
    if(slab->prev_slab) slab->prev_slab->next_slab = slab->next_slab;
    if(slab->next_slab) slab->next_slab->prev_slab = slab->prev_slab;
   slab->prev_slab = nullptr;

    switch(flag){
        case ALLOCATE: {
          if(slab->free_object_count == 0) {
                *slab->list_head = (*slab->list_head)->next_slab;
                if(full_slabs) full_slabs->prev_slab = slab;
                slab->next_slab = full_slabs;
                full_slabs = slab;
                slab->list_head = &full_slabs;
            } else if(slab->free_object_count == slab->max_object_count -1){
                *slab->list_head = (*slab->list_head)->next_slab;
                if(used_slabs) used_slabs->prev_slab = slab;
                slab->next_slab = used_slabs;
                used_slabs = slab;
                slab->list_head = &used_slabs;
            }
            break;
        }
        case FREE:
        {
            if(slab->free_object_count == 1){
                if(slab == full_slabs) *slab->list_head = (*slab->list_head)->next_slab;
                if(used_slabs) used_slabs->prev_slab = slab;
                slab->next_slab = used_slabs;
                used_slabs = slab;
                slab->list_head = &used_slabs;
            } else if(slab->free_object_count == slab->max_object_count){
                if(slab == used_slabs || slab == full_slabs) *slab->list_head = (*slab->list_head)->next_slab;
                if(free_slabs) free_slabs->prev_slab = slab;
                slab->next_slab = free_slabs;
                free_slabs = slab;
                slab->list_head = &free_slabs;
            }
            break;
        }
    }
}

kSlab *kCache::find_slab(kSlab *head, void* obj) {
    kSlab* cur, *target = nullptr;
    for(cur = head; cur; cur = cur->next_slab){
        if((char*)cur->start < (char*)obj && (char*)cur->end > (char*)obj){
            target = cur;
            return target;
        }
    }
    return nullptr;
}


void kCache::init(bool off, const char *name, size_t obj_size, size_t sl_size,void (*ctor)(void *), void (*dtor)(void *)) {
    free_slabs = full_slabs = used_slabs = nullptr;
    off_slab = off; object_size = obj_size; cnstor = ctor; dstor = dtor;
    slab_size = sl_size;
    int i =0; while((*name) != '\0' && i<CACHE_NAME_SIZE-1) cacheName[i++]=*name++;
    cacheName[CACHE_NAME_SIZE - 1]='\0';
    next = head;
    if(head) head->prev = this;
    head = this;
    prev = nullptr;
    to_shrink = 0;
    error = num = 0;
    obj_num = slab_size/(sizeof (FreeObject) + obj_size + (off_slab? 0 : sizeof (kSlab)));

}

void kCache::_init(void *p) {
    new(p) kCache();
}

void kCache::_deinit(void *p) {
    ((kCache*)p)->~kCache();
}

void *kCache::operator new(size_t sz, void *adr) {
    return adr;
}

int kCache::shrink() {
    if(to_shrink < 2) {
        delete_slabs(free_slabs);
        to_shrink = 1;
    }
    return 0;
}

void kCache::delete_slab(kSlab *slab) {
    num--;
    //remove from list
    if(slab == *slab->list_head) *slab->list_head = (*slab->list_head)->next_slab;
    if(slab->next_slab)slab->next_slab = slab->prev_slab;
    if(slab->prev_slab)slab->prev_slab = slab->next_slab;

    //deallocate using Buddy functions
    void* base = off_slab ? slab->start : ((char*)slab->start - sizeof(kSlab));
    slab->deinit();
    BuddyAllocator::free(base, slab_size);
}

void kCache::delete_slabs(kSlab* head){
    kSlab* cur = head;
    while(cur){
        kSlab* s = cur;
        cur = cur->next_slab;
        delete_slab(s);
    }
}

void kCache::deinit() {
    if(this == head) head = this->next;
    if(this->prev) prev->next = this->next;
    if(this->next) next->prev = this->prev;
    delete_slabs(free_slabs);
    delete_slabs(used_slabs);
    delete_slabs(full_slabs);
}

int kCache::get_size() {
    return object_size;
}

int kCache::get_slabs() {
    return num;
}

int kCache::get_object_num() {
    return obj_num;
}

int kCache::get_percent() {
    if (num == 0) return 0;
    int n = num * obj_num; //max
    kSlab* cur;
    int c = 0;
    for(cur = used_slabs; cur; cur = cur->next_slab) {
        c+=(cur->max_object_count - cur->free_object_count);
    }
    for(cur = full_slabs; cur; cur = cur->next_slab) c+=obj_num;
    return (100*c + n/2)/n;
}

char *kCache::getName() {
    return cacheName;
}

int kCache::get_error() {
    return error;
}


