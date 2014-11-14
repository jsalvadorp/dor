//#include <gc/gc.h>
//#include <gc/gc_cpp.h>
//#include <gc/gc_allocator.h>

#include <algorithm>

#include "loader.hpp"
#include "heap.hpp"

obj::word_t *makeClosure(Proc *proc, obj::word_t *data, uint64_t size) {
    obj::word_t *clos = new obj::word_t[size + 2];
    clos[0].ui64 = size;
    clos[1].ptr = (void *) proc;
    //memcpy(clos + 2, data, size * sizeof(obj::word_t));
    std::copy(data, data + size, clos + 2);
    return clos;
}

obj::word_t *makeChunk(obj::word_t *data, uint64_t size) {
    obj::word_t *chunk = new obj::word_t[size];
    //memcpy(chunk, data, size * sizeof(obj::word_t));
    std::copy(data, data + size, chunk);
    return chunk;
}

obj::word_t *extendClosure(obj::word_t *closure, obj::word_t *data, uint64_t size) {
    int old_size = CLOS_SIZE(closure);
    obj::word_t *new_clos = new obj::word_t[2 + old_size + size];
    
    // write the new closure size
    new_clos[0].ui64 = old_size + size;
    
    // copy the proc address and the old arguments
    std::copy(closure + 1, closure + 2 + old_size, new_clos + 1);

    // copy the new arguments
    std::copy(data, data + size, new_clos + 2 + old_size);

    return new_clos;
}

obj::word_t *allocate(size_t size) { // size in words
    return new obj::word_t[size];
}
