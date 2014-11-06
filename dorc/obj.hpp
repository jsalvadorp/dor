#pragma once

#include <cstdint>
#include <fstream>

namespace obj {

union word_t {
    uint64_t ui64;
    double f64;
    char c[8];
};

struct Header {
    enum {
        size = 32
    };

    uint32_t pool_start;
    uint32_t data_start;

    uint32_t pool_count;
    uint32_t extern_count;
    uint32_t data_size;
    
    uint32_t load;
    uint32_t entry;
    uint32_t unknown;
};

struct ExternEntry {
    enum {
        size = 0
    };
    // unknown
};

enum DataType {
    DataChar, 
    DataFloat64, DataFloat32, 
    DataInt64, DataInt32, DataInt16, DataInt8,
    DataProc,
    DataNotPresent // to be loaded later
};

struct PoolEntry {
    enum {
        size = 16
    };
    uint32_t type, length;
    word_t value;

    PoolEntry() { value.ui64 = type = length = 0;}
};

template <typename T>
void writeVal(std::ofstream &out, T t, bool swap_endianness = false) {
    // do something with byte order!

    out.write(reinterpret_cast<const char *>(&t), sizeof(T));
}

void writeCode(std::ofstream &out, std::vector<unsigned char> &code) {
    const char pad = 0;
    out.write((const char *)code.data(), code.size());
    for(int i = code.size(); i % 8; i++) out.write(&pad, 1);
}

void writeString(std::ofstream &out, std::string &code) {
    const char pad = 0;
    out.write((const char *)code.data(), code.size());
    for(int i = code.size(); i % 8; i++) out.write(&pad, 1);
}


}
