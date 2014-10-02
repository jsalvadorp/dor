#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>

#include "symbol.hpp"

namespace ast {

template <typename T>
using Ptr = T*;

using std::string;


typedef int64_t i64;
typedef int uchar;

// Variant type using polymorphism
// Consider implementing this solution with templates, 
// visitor pattern perhaps, boost::variant

enum Type {
	ListType = 0,
	StringType,
	I64Type,
	CharType,
	DoubleType,
	BoolType,
	SymType,
    VoidType
};

struct List;
struct AVoid;

inline void print_indent(int level) {
    //assert(level < 4);
    while(level--) std::cout << "  ";
}

struct Atom {
    int line;
    int column;
    Atom() : line(-1), column(-1) {}
    Atom(int l, int c) : line(l), column(c) {}
    virtual Type type() = 0;
    virtual ~Atom() {}
    
    virtual void dump(int indent) = 0;
    
    // literals
    virtual i64 get_i64() {assert(!"Not an int64"); return 0;}
    virtual int get_char() {assert(!"Not a char"); return 0;}
    virtual double get_double() {assert(!"Not a double"); return 0;}
    virtual bool get_bool() {assert(!"Not a bool"); return 0;}
    virtual std::string get_string() {assert(!"Not a string"); return std::string();}
    //virtual void get_void() {assert(!"Not a void"); return;}
    
    // syms lists
    virtual Sym get_Sym() {assert(!"Not a symbol"); return Sym();}
    virtual List *get_List() {assert(!"Not a list"); return nullptr;}
};

struct List : Atom { // never-empty list
    Ptr<Atom> head;
    Ptr<List> tail;
    
    virtual Type type() {return ListType;}
    
    virtual List *get_List() {
        return this;
    }
    
    virtual ~List() {}
    
    virtual void dump(int level) {
        print_indent(level);
        std::cout << "(" << std::endl;
        for(Atom *a : *this) {
            a->dump(level + 1);
            std::cout << std::endl;
        }
        print_indent(level);
        std::cout << ")";
    }
    
    //List() : head(Atom::VoidValue), tail(nullptr) {}
    List(Atom *head, List *list) : head(head), tail(list) {}
    template<typename T>
    List(T head, List *list); //: head(head), tail(list) {}
    
    struct List_iterator {
        
        List_iterator &operator++() {el = el->tail;}
        List_iterator &operator++(int x) {el = el->tail;}
        Atom *operator*() {return el->head;}
        Atom *operator->() {return el->head;}
        bool operator==(List_iterator rhs) {return el == rhs.el;}
        bool operator!=(List_iterator rhs) {return el != rhs.el;}
        
        
        Ptr<List> el;
        
        List_iterator(List *rhs) {el = rhs;}
        List_iterator(const List_iterator &rhs) {el = rhs.el;}
        
    };
    
    typedef List_iterator iterator;
    
    iterator begin() {return iterator(this);}
    iterator end() {return iterator(nullptr);}
};

List **append(List **l, Atom *a);

#define ATOM_SPECIALIZE(name, ty) \
struct A##name : Atom { \
    ty value; \
    A##name(ty t) : value(t) {} \
    A##name(ty t, int l, int c) : value(t), Atom(l, c) {} \
    virtual Type type() {return name##Type;} \
    virtual ty get_##ty() { \
        return value; \
    } \
    virtual ~A##name() {} \
    virtual void dump(int level) { print_indent(level); std::cout << get_##ty();} \
}; \
inline Atom *atom(ty t, int l, int c) {return new A##name(t, l, c);}\
inline Atom *atom(ty t) {return new A##name(t);}

inline std::ostream &operator<<(std::ostream &out, Sym r) {
    std::cout << r.str();
}

ATOM_SPECIALIZE(I64, i64);
ATOM_SPECIALIZE(Bool, bool);
ATOM_SPECIALIZE(Double, double);
ATOM_SPECIALIZE(Char, uchar);
ATOM_SPECIALIZE(Sym, Sym);
ATOM_SPECIALIZE(String, string);

inline Atom *atom(char c) {return atom((uchar)c);}
inline Atom *atom(List *l) {return (Atom *)l;}

struct AVoid : Atom {
    virtual Type type() {return VoidType;}
    virtual ~AVoid() {}
    virtual void dump(int level) {print_indent(level); std::cout << "v()" << std::endl;}
    static AVoid *value();
    private:
    AVoid() {}
};

extern AVoid *voidv;


/*
void func() {
    for(Atom *a : List()) {}
}/**/


template<typename T>
List::List(T head, List *list) : head(atom(head)), tail(list) {}

}



#if 0

ASTAtom Alc(ASTAtom, int, int);

ASTAtom Along(long x);
ASTAtom Acstring(cstring x); 
ASTAtom Adouble(double x);
ASTAtom Abool(bool x); 
ASTAtom Achar(long x); 
ASTAtom ASymbol(Symbol x); 
ASTAtom Asymbol(cstring x); 
bool ASTAtom_isNil(ASTAtom x); 

ASTAtom AList(ASTList * c);
ASTAtom AASTList(ASTList * c);
ASTList * ASTList_new(ASTAtom car, ASTList * next);
ASTList * ASTList_add(ASTList ** l, ASTAtom n);
ASTList * ASTList_append(ASTList ** l, ASTList * l2);
ASTList * ASTList_list(size_t count, ...);
ASTList * ASTList_last(ASTList * l);
ASTList * ASTList_reversed(ASTList * l);
ASTAtom * ASTList_at(ASTList * this, size_t index);
size_t ASTList_length(ASTList * this);
ASTList * ASTList_next(ASTList * c);
ASTList ** ASTList_nextr(ASTList * c);

#define foreach(el, li) for(ASTList * el = li; el; el = el->next)

extern ASTAtom nil;

void to_str(char * buf, ASTAtom a);
void dump(ASTAtom what, int indent); 

#endif
