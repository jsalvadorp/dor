#pragma once

#include <unordered_map>
#include <vector>
#include <initializer_list>
#include "ast.hpp"
#include "types.hpp"

using ast::Ptr;
using ast::Atom;

enum scope_t {
    EXTERN,
    GLOBAL, 
    PARAMETER, 
    LOCAL, 
    CLOSURE
};

enum mutability_t {
    CONST, // compile-time constant
    FINAL, // immutable
    VAR    // mutable
};

enum storage_t {
    AUTO, // stack variables
    STATIC // 
};

struct Type;
struct Expression;

// DO NOT Ptr<Binding> !!!
struct Binding {
    scope_t scope;
    mutability_t mut;
    storage_t storage;    
    
    Ptr<Type> type;
    Sym name;
    Sym qualified_name;
    
    Ptr<Expression> value;
    Ptr<Binding> parent;
    
    Binding() {}
    Binding(const Binding &b)
        : scope(b.scope)
        , parent(b.parent)
        , type(b.type)
        , name(b.name)
        , qualified_name(b.qualified_name)
        , value(b.value) {}
    Binding(Binding &parent, scope_t scope) 
        : scope(scope)
        , parent(&parent)
        , type(parent.type)
        , name(parent.name)
        , qualified_name(parent.qualified_name)
        , value(parent.value) {}
};

struct Env {
    Ptr<Env> parent;
    std::unordered_map<Sym, Binding> dictionary;
    
    virtual Binding *find(Sym name);
};

struct Globals : Env {
    std::vector<Binding *> externs; 
    // handle externs
    virtual Binding *find(Sym name);
};

struct Expression {
    int line, column;
    Ptr<Type> type;
};

struct Function : Expression, Env {
    std::vector<Binding *> closure;
    std::vector<Binding *> parameters; 
    virtual Binding *find(Sym name);
    
    Ptr<Expression> body;
};

struct Literal : Expression {
    Ptr<Atom> value;
};

struct Reference : Expression {
    Binding *binding;
};

struct Application : Expression {
    Ptr<Expression> left, right;
};

struct Conditional : Expression {
    Ptr<Expression> condition, on_false, on_true;
};

struct Sequence : Expression {
    Ptr<Env> locals;
    std::vector<Ptr<Expression> > steps;
};

struct MatchClause {
    Ptr<Env> env;
    Ptr<Expression> pattern, body;
};

struct Match : Expression {
    std::vector<MatchClause> clauses;
};



// assignment
