#ifndef NADEKOSENGOKU_KCACHE_H
#define NADEKOSENGOKU_KCACHE_H

#include "kSlab.hpp"
//Cache Descriptor Class
#define SIZE_N 13
#define CACHE_NAME_SIZE 32
#define BAD_ALLOC 1
#define BAD_FREE 2

class kCache {
public:
    void*  allocate_object();
    int free_object(void*);
    int shrink();
    char* getName();
    int get_size();
    int get_slabs();
    int get_object_num();
    int get_percent();
    int get_error();

    kCache(){

    }
    ~kCache(){

    }


    void* operator new(size_t sz, void* adr);

    static kCache* size_N[SIZE_N]; //12 Caches for small memory buffers
    static kCache *cache_cache, *slab_cache;//cache_cache off_slab = false

    static kCache* head; //head of the cache List, init to nullptr
    kCache *prev, *next; //Linking the Cache

    static void _init(void *);
    static void _deinit(void*);
    void init(bool off, const char* name, size_t size, size_t sl_size, void(*ctor)(void*),void (*dtor)(void *));
    void deinit();

protected:
    kSlab *find_free_slab(kSlab* head); //find slab where to allocate the object
    void *allocate_slab();
    void reposition(kSlab*, int flag);
    kSlab *find_slab(kSlab* head, void*);
    void delete_slabs(kSlab *head);
    void delete_slab(kSlab*);
private:

    bool off_slab;
    int to_shrink;
    size_t object_size, slab_size;
    Func cnstor, dstor;


    char cacheName[CACHE_NAME_SIZE];//cache name size

    kSlab *free_slabs, *full_slabs, *used_slabs;
    int num; //number of slabs
    int obj_num; //number of objects on one slab
    int error; //for error codes
};

#endif //NADEKOSENGOKU_KCACHE_H
