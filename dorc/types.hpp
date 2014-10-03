#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>

#include "ast.hpp"

using ast::Ptr;

struct Kind {
    Ptr<Kind> from;
    Ptr<Kind> to;
    
    //static Ptr<Kind> k0();
    
    template<int N>
    static Ptr<Kind> k() {
        static Ptr<Kind> kind = newPtr<Kind>(k<0>(), k<N - 1>());
        return kind;
    }
    
    Kind(Ptr<Kind> f, Ptr<Kind> t) : from(f), to(t) {}
};

template<>
inline Ptr<Kind> Kind::k<0>() {
    return nullptr;
}



#define K1 Kind::k<1>()
#define K2 Kind::k<2>()
#define K3 Kind::k<3>()
#define K4 Kind::k<4>()

struct Type {
    Ptr<Kind> kind;
    Sym name;
    
    Type(Ptr<Kind> kind = nullptr) : kind(kind) {}
    Type(Sym name, Ptr<Kind> kind = nullptr) : name(name), kind(kind) {}
    Type(const char *s, Ptr<Kind> kind = nullptr) : name(Sym(s)), kind(kind) {}
};

struct TypeApp : Type {
    Ptr<Type> left;
    Ptr<Type> right;
    
    TypeApp(Ptr<Type> left, Ptr<Type> right);
};

extern Ptr<Type> Int, String, Char, Bool, Void, Float, FuncArrow;

#if 0


typedef struct {
	type_t t;
	
	qtype_t * q;
	int id;
} tvar_t;


typedef struct {
	type_t t;
	
	tvar_t var;
	type_t * body;
} qtype_t;

typedef struct {
	int n;
	type_t ** nt;
} typel_t;

// for each compiled function, show up to which parameter it is compiled

typedef struct {
	type_t t;
	
	type_t * takes;
	type_t * rets;
} funct_t;
#define FUNCT 0

typedef struct {
	type_t t;
	
	int * cnsizes;
	int * cnconsts;
	
	int c0; // number of nullary constructors
	int cn;
} adt_t;
#define ADT 1

typedef struct {
	type_t t;
	int cs;
	
	typel_t els;
} tuple_t;
#define TUPLE 2

typedef struct {
	type_t t;
	
	type_t * pointed;
} tvar_t;
#define TVAR 99

typedef struct {
	type_t t;
	
	tenv_t * env;
	type_t * exp;
} quantified_t;
#define QUANT 100

typedef type_t * typeptr;

define_hmap_interface(Symbol, typeptr);

typedef struct tenv {
	hmap_Symbol_typeptr dict;
	struct tenv * parent;
} tenv_t;

type_t * funct(type_t * takes, type_t * rets);

type_t * cfunct(tenv_t * env, ASTList * body);
type_t * cadt(tenv_t * env, ASTList * body);
type_t * ctuple(tenv_t * env, ASTList * body);
type_t * cpmap(tenv_t * env, ASTList * body);

extern kind_t * kind0;
extern tenv_t * globalt;
#endif
