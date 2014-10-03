#pragma once

#include <unordered_map>
#include <vector>
#include <initializer_list>
#include "ast.hpp"

using ast::Ptr;
using ast::Atom;

typedef enum {
    EXTERN,
    GLOBAL, 
    PARAMETER, 
    LOCAL, 
    CLOSURE
} scope_t;

struct Type;
struct Expression;

// DO NOT Ptr<Binding> !!!
struct Binding {
    scope_t scope;
    Ptr<Type> type;
    Sym name;
    Sym qualified_name;
    
    Ptr<Expression> value;
    Ptr<Binding> parent;
    
    bool variable; // handle mutability later
    
    Binding() {}
    Binding(const Binding &b)
        : scope(b.scope)
        , parent(b.parent)
        , type(b.type)
        , name(b.name)
        , qualified_name(b.qualified_name)
        , value(b.value)
        , variable(b.variable) {}
    Binding(Binding &parent, scope_t scope) 
        : scope(scope)
        , parent(&parent)
        , type(parent.type)
        , name(parent.name)
        , qualified_name(parent.qualified_name)
        , value(parent.value)
        , variable(parent.variable) {}
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

struct Application : Expression {
    Ptr<Expression> left, right;
};

struct Conditional : Expression {
    Ptr<Expression> condition, on_false, on_true;
};

struct Literal : Expression {
    Ptr<Atom> value;
};

struct Reference : Expression {
    Binding *binding;
};

// assignment
