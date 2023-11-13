#ifndef NADEKOSENGOKU_KSLAB_H
#define NADEKOSENGOKU_KSLAB_H
#include "../lib/hw.h"


struct FreeObject{
    FreeObject* next_object;
    void* owner;
};

using Func=void(*)(void*);

class kCache;
class kSlab {
public:
    void* allocate();
    int free(void* obj);
    kSlab(){

    }
    ~kSlab(){

    }

    kSlab(void* adr, size_t obj_size, size_t size, kSlab *prev, Func ctor, Func dtor, kCache* o):
    cnstor(ctor), dstor(dtor){
        init(adr, obj_size, size, prev, ctor, dtor, o);
    }

    static void _init(void*);
    static void _deinit(void*);
    void init(void* adr, size_t obj_size, size_t size, kSlab *next, Func ctor, Func dtor, kCache*);
    void deinit();
    void* operator new(size_t sz, void* adr);
    kCache* owner;

private:
    kSlab  *prev_slab, *next_slab;
    kSlab **list_head;
    Func cnstor, dstor;

    FreeObject* head; //how to handle initialized objects??
    void *start, *end; //slab beginning and end, do i need this? Probably not.
    size_t object_size, free_object_count, max_object_count; //fixed object size
    friend class kCache;

};


#endif //NADEKOSENGOKU_KSLAB_H
