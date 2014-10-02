#include <unordered_map>
#include <vector>
#include <string>
#include "symbol.hpp"

std::unordered_map<std::string, int> str2sym;
std::vector<std::string> sym2str;
//std::vector<const std::string *> sym2str; // avoid duplication, use pointers

Sym::Sym(const char *s) {
    auto it = str2sym.find(s);
    
    if(it == str2sym.end()) {
        code = str2sym[s] = sym2str.size();
        sym2str.push_back(s);
    } else {
        code = str2sym[s];
    }
}
Sym::Sym(const std::string &s) {
    auto it = str2sym.find(s);
    
    if(it == str2sym.end()) {
        code = str2sym[s] = sym2str.size();
        sym2str.push_back(s);
    } else {
        code = str2sym[s];
    }
}

const std::string &Sym::str() {
    //return *sym2str[code];
    return sym2str[code];
}
