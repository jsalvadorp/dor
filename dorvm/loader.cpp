#include <cassert>
#include <cstring>
#include <iostream>

#include "loader.hpp"
#include "heap.hpp"

#include <iomanip>
#include "../dorc/instructions.hpp"

Proc *makeProc(char *buf, size_t length, obj::word_t *pool); 

Pool loadBinary(std::ifstream &in) {
    Pool p;
    char c;

    while(in.get(c) && c != '\n');

    assert(in);

    while(in.tellg() % 8 && in.get(c));

    assert(in);

    uint64_t bom;
    obj::readVal(in, bom);

    obj::Header header;
    readVal(in, header);

    p.load = header.load;
    p.entry = header.entry;


        std::vector<obj::PoolEntry> cpool(header.pool_count);

    // read extern info

    // read constant pool
    for(int i = 0; i < header.pool_count; i++) {
        obj::PoolEntry pe;
        readVal(in, pe);
        cpool[i] = pe;
    }
    
    //pool = std::vector<obj::word_t>(header.extern_count + header.pool_count);
    size_t global_count = header.extern_count + header.pool_count;
    p.pool = new obj::word_t[global_count];
    p.size = global_count;
    //std::cout << "externs " << header.extern_count << " interns " << header.pool_count << std::endl;
    memset(p.pool, 0, global_count * sizeof(obj::word_t));

    in.seekg(header.data_start);
    
    size_t sizes[20] = {0};
    sizes[obj::DataChar] = sizes[obj::DataInt8] = 1;
    sizes[obj::DataFloat64] = sizes[obj::DataInt64] = 8;
    sizes[obj::DataInt32] = 4;
    sizes[obj::DataInt16] = 2;

    for(int i = 0; i < header.pool_count; i++) {
        //in.read(&cpool[header.extern_count], sizeof(obj::PoolEntry));
        if(cpool[i].type == obj::DataChar && cpool[i].length) { // string
            char *buf = new char[cpool[i].length];
            in.read(buf, cpool[i].length);
            obj::readPadding(in, cpool[i].length); 
            //cpool[header.extern_count + i].value.ptr = makeString(buf, cpool[i].length);
            p.pool[header.extern_count + i].ptr = makeString(buf, cpool[i].length);
            delete[] buf;
        } else if(cpool[i].type == obj::DataProc) {
            char *buf = new char[cpool[i].length];
            in.read(buf, cpool[i].length);
            obj::readPadding(in, cpool[i].length); 
            Proc *proc = makeProc(buf, cpool[i].length, p.pool);
            p.pool[header.extern_count + i].ptr = (void *) makeClosure(proc, nullptr, 0);
            delete[] buf;
        } else if(cpool[i].type == obj::DataNotPresent) {
            
        } else {
            assert(cpool[i].length == 0); // no other kinds of heap data supported at the moment
            // readVal(in, pool[i]);
            p.pool[header.extern_count + i] = cpool[i].value;
        }
    }

    return p;
}

#define TO8(x) (((x) + 7) & ~(7))

void *makeString(const char *buf, size_t length) {
    size_t size = 16 + TO8(length);
    char *news = new char[size];
    *((uint64_t *) (news + size - 16)) = 0; // zero-terminate
    *((uint64_t *) (news + size - 8)) = 0; // zero-terminate
    *((uint64_t *) news) = length; // write length in bytes
    memcpy(news + 8, buf, length);

    assert(news);
    return (void *) news;
}

Proc *makeProc(char *buf, size_t length, obj::word_t *pool) {
    Proc *newp = new Proc;
    assert(length > 8);
    newp->arity = ((uint32_t *) buf)[0];
    newp->locals = ((uint32_t *) buf)[1];
    
    unsigned char *code = new unsigned char[length - 8];
    memcpy(code, buf + 8, length - 8);
    newp->pool = pool;
    newp->code = code;


    return newp;
}
