#include "../h/kSlab.hpp"

void *kSlab::allocate() {
    if(free_object_count == 0 ) return nullptr;
    free_object_count--;

    void* adr = (char*)head + sizeof (FreeObject);

    FreeObject* old_head = head;
    head = head->next_object;
    old_head->next_object = nullptr;

    return adr;
}

int kSlab::free(void *obj) {
    if(obj == nullptr
    || free_object_count==max_object_count) return -1;

    free_object_count++;
    FreeObject* node = (FreeObject*)((char*)obj - sizeof(FreeObject));

    node->next_object = head;
    head = node;

    return 0;
}

void kSlab::init(void *adr, size_t obj_size, size_t size, kSlab *next, Func ctor, Func dtor, kCache* o) {

    prev_slab = nullptr; next_slab = next;
    if(next_slab) list_head = next_slab->list_head;
    cnstor = ctor; dstor = dtor;
    object_size = obj_size;
    head = nullptr;
    start = adr;
    free_object_count = max_object_count = size/(object_size + sizeof (FreeObject));
    FreeObject *cur, *previous = nullptr;
    for(size_t i = 0; i < free_object_count; i++){
        cur = (FreeObject*) adr;
        cur->next_object= nullptr;
        cur->owner = this;
        if(head == nullptr) head = cur;
        if(previous != nullptr) previous->next_object = cur;
        adr = (char*)adr + sizeof (FreeObject);
        if(this->cnstor) this->cnstor(adr);
        adr = (char*)adr + object_size;
        previous = cur;
    }
    end = adr;
    owner = o;
}

void kSlab::_init(void *adr) {
    new(adr) kSlab();
}

void *kSlab::operator new(size_t sz, void *adr) {
    return adr;
}

void kSlab::_deinit(void *obj) {
    ((kSlab*)obj)->~kSlab();
}

void kSlab::deinit() {
    void* adr = start;
    FreeObject* cur;
    for(size_t i =0; i<max_object_count; i++){
        cur = (FreeObject*) adr;
        cur->next_object = nullptr;
        cur->owner = nullptr;
        adr = (char*)adr + sizeof (FreeObject);
        if(this->dstor) this->dstor(adr);
        adr = (char*)adr + object_size;
    }
    start = end = nullptr;
    owner = nullptr;
    head = nullptr;
    max_object_count = free_object_count = 0;
}


