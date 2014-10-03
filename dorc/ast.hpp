#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>
#include <memory>

#include "symbol.hpp"

namespace ast {

template <typename T>
using Ptr = std::shared_ptr<T>;//T*;

template <typename T>
using WeakPtr = std::weak_ptr<T>;//T*;

#define newPtr std::make_shared
//Ptr<T> newPtr(T *t) {return std::make_shared<T>(t);}

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

struct Atom : std::enable_shared_from_this<Atom> {
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
    //virtual Ptr<List> get_List() {assert(!"Not a list"); return nullptr;}
};

struct List : Atom { // never-empty list
    Ptr<Atom> head;
    Ptr<List> tail;
    
    virtual Type type() {return ListType;}
    
    /*virtual Ptr<List> get_List() {
        return shared_from_this();
    }*/
    
    virtual ~List() {}
    
    virtual void dump(int level) {
        print_indent(level);
        std::cout << "(" << std::endl;
        for(Ptr<Atom> a : *this) {
            a->dump(level + 1);
            std::cout << std::endl;
        }
        print_indent(level);
        std::cout << ")";
    }
    
    Ptr<List> copy() {
        return newPtr<List>(head, tail ? tail->copy() : nullptr);
    }
    
    //List() : head(Atom::VoidValue), tail(nullptr) {}
    List(Ptr<Atom> head, Ptr<List> list) : head(head), tail(list) {}
    template<typename T>
    List(T head, Ptr<List> list); //: head(head), tail(list) {}
    
    struct List_iterator {
        
        List_iterator operator++() {el = el->tail.get(); return *this;} //pre
        List_iterator operator++(int x) { // post increment
            List_iterator it(*this);
            el = el->tail.get();
            return it;
        } //post
        Ptr<Atom> operator*() {return el->head;}
        Ptr<Atom> operator->() {return el->head;}
        bool operator==(List_iterator rhs) {return el == rhs.el;}
        bool operator!=(List_iterator rhs) {return el != rhs.el;}
        
        
        List *el;
        
        List_iterator(List *rhs) {el = rhs;}
        List_iterator(const List_iterator &rhs) {el = rhs.el;}
        
    };
    
    typedef List_iterator iterator;
    
    iterator begin() {return iterator(this);}
    iterator end() {return iterator(nullptr);}
};

Ptr<List> list(std::initializer_list<Ptr<Atom> > l);
//Ptr<List> list(std::initializer_list<Ptr<Atom> > l);


inline Ptr<Atom> atom(Ptr<List> &l) {return std::dynamic_pointer_cast<Atom, List>(l);}
inline Ptr<List> asList(const Ptr<Atom> &r) {
    assert(r->type() == ListType);
    
    return std::dynamic_pointer_cast<List, Atom>(r);
}
Ptr<List> *append(Ptr<List> *l, Ptr<Atom> a);
Ptr<List> *appendList(Ptr<List> *l, Ptr<List> a);

#define ATOM_SPECIALIZE(name, ty) \
struct A##name : Atom { \
    ty value; \
    A##name(const ty &t) : value(t) {} \
    A##name(const ty &t, int l, int c) : value(t), Atom(l, c) {} \
    virtual Type type() {return name##Type;} \
    virtual ty get_##ty() { \
        return value; \
    } \
    virtual ~A##name() {} \
    virtual void dump(int level) { print_indent(level); std::cout << get_##ty();} \
}; \
inline Ptr<Atom> atom(ty t, int l, int c) {return newPtr<A##name>(t, l, c);}\
inline Ptr<Atom> atom(ty t) {return newPtr<A##name>(t);}

inline std::ostream &operator<<(std::ostream &out, Sym r) {
    return std::cout << r.str();
}

ATOM_SPECIALIZE(I64, i64);
ATOM_SPECIALIZE(Bool, bool);
ATOM_SPECIALIZE(Double, double);
ATOM_SPECIALIZE(Char, uchar);
ATOM_SPECIALIZE(Sym, Sym);
ATOM_SPECIALIZE(String, string);


inline Ptr<Atom> atom(char c) {return atom((uchar)c);}

struct AVoid : Atom {
    virtual Type type() {return VoidType;}
    virtual ~AVoid() {}
    virtual void dump(int level) {print_indent(level); std::cout << "v()" << std::endl;}
    static Ptr<AVoid> value();
    AVoid() {}
    AVoid(const AVoid &r) {}
};



template<typename T>
List::List(T head, Ptr<List> list) : head(atom(head)), tail(list) {}

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
