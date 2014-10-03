#pragma once

#include <unordered_map>
struct Sym {
    int code;
    
    Sym(int i) : code(i) {}
    Sym(const char *s = "");
    Sym(const std::string &s);
    const std::string &str() const;
    bool operator==(const Sym s) const {return code == s.code;}
    bool operator!=(const Sym s) const {return code != s.code;}
    
    
    bool operator==(const std::string &s) const {return str() == s;}
    bool operator!=(const std::string &s) const {return str() != s;}
    bool operator==(const char *s) const {return str() == s;}
    bool operator!=(const char *s) const {return str() != s;}
    
    bool valid() {return code >= 0;}
};

namespace std {
template <>
struct hash<Sym> {
    std::size_t operator()(const Sym &k) const {
        return std::hash<int>()(k.code);
    }
};
};
