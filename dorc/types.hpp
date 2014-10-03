#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>

#include "ast.hpp"

using ast::Ptr;

struct Kind {
    Ptr<Kind> from;
    Ptr<Kind> to;
    
    static Ptr<Kind> k0();
    
    Kind(Ptr<Kind> f, Ptr<Kind> t) : from(f), to(t) {}
};

#define STAR Kind::k0()

struct Type {
    Ptr<Kind> kind;
    
    Type() : kind(nullptr) {}
};

struct TypeApp : Type {
    Ptr<Type> left;
    Ptr<Type> right;
    
    TypeApp(Ptr<Type> left, Ptr<Type> right);
};



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
