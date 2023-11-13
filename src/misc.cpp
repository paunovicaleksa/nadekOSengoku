#include "../h/misc.h"


int log2_64(size_t sz) {
    size_t ceil = ceil_2(sz);
    int k =0;
    size_t n = 1;
    while(n < ceil){
        k++;
        n*=2;
    }
    return k;
}

//find next power of 2 for sz
size_t ceil_2(size_t sz) {
    if (sz == 0) return 0;

    sz--;

    sz |= sz >> 1;
    sz |= sz >> 2;
    sz |= sz >> 4;
    sz |= sz >> 8;
    sz |= sz >> 16;
    sz |= sz >> 32;

    sz++;

    return sz;
}

void memset(const void *data, char val, int size) {
    for(int i =0; i<size; i++){
        ((char*)data)[i] = val;
    }
}
