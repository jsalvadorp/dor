#pragma once

#include "../dorc/obj.hpp"

struct Pool {
    uint32_t load, entry;

    obj::word_t *pool;
    Pool() : pool(nullptr) {}
    //~Pool() {if(pool) delete[] pool;}
};

struct Proc {
    uint32_t arity;
    uint32_t locals;
    obj::word_t *pool;
    const unsigned char *code;
    Proc() : code(nullptr) {}
    //~Proc() {if(code) delete [] code;}
};

void *makeString(const char *buf, size_t length);

Pool loadBinary(std::ifstream &in);
