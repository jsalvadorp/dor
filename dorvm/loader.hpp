#pragma once

#include "../dorc/obj.hpp"

typedef obj::word_t NativeFunc (obj::word_t *args);

struct Pool {
    uint32_t load, entry;

    uint64_t size;

    obj::word_t *pool;
    Pool() : pool(nullptr) {}
    //~Pool() {if(pool) delete[] pool;}
};

struct Proc {
    uint32_t arity;
    uint32_t locals;
    obj::word_t *pool;
    NativeFunc *native_impl;
    const unsigned char *code;
    Proc() : code(nullptr), native_impl(nullptr) {}
    Proc(NativeFunc *f, int _arity) : native_impl(f), arity(_arity), locals(0) {}
    //~Proc() {if(code) delete [] code;}
};

void *makeString(const char *buf, size_t length);

Pool loadBinary(std::ifstream &in);
