#pragma once

#include <cstdint>
#include <cassert>

namespace ast {

template <typename T>
using Ptr = T*;

struct Sym {};

typedef int64_t i64;

enum Type {
	ListType = 0,
	StringType,
	LongType,
	CharType,
	DoubleType,
	BoolType,
	SymbolType
};

struct List;


struct Atom {
    int line;
    int column;
    Atom() : line(-1), column(-1) {}
    Atom(int l, int c) : line(l), column(c) {}
    virtual Type type() = 0;
    virtual ~Atom() = 0;
    
    
    // literals
    virtual i64 get_i64() {assert(!"Not an int64"); return 0;}
    virtual int get_char() {assert(!"Not a char"); return 0;}
    virtual double get_double() {assert(!"Not a double"); return 0;}
    virtual bool get_bool() {assert(!"Not a bool"); return 0;}
    virtual std::string get_string() {assert(!"Not a string"); return std::string();}
    
    // syms lists
    virtual Sym get_Sym() {assert(!"Not a symbol"); return Sym();}
    virtual Ptr<List> get_List() {assert(!"Not a list"); return nullptr;}
};

struct List {
    Ptr<Atom> head;
    Ptr<List> tail;
    
    List() : head(nullptr), tail(nullptr) {}
};

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
