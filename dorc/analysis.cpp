#include <unordered_map>
#include "analysis.hpp"

using namespace ast;

Binding *Env::find(Sym name) {
    auto it = dictionary.find(name);
    Binding *b;
    
    if(it != dictionary.end()) return &it->second;
    else if(parent && (b = parent->find(name))) {
        return b;
    }
    
    return nullptr;
}

Binding *Globals::find(Sym name) {
    auto it = dictionary.find(name);
    Binding *b;
    
    if(it != dictionary.end()) return &it->second;
    /*else if(parent && (b = parent->find(name))) {
        return b;
    }*/ // handle externs!!!
    
    return nullptr;
}

Binding *Function::find(Sym name) {
    auto it = dictionary.find(name);
    Binding *b;
    
    if(it != dictionary.end()) return &it->second;
    else if(parent && (b = parent->find(name))) {
        if(b->scope == PARAMETER || b->scope == CLOSURE) {
            dictionary[name] = Binding(*b, CLOSURE);
        }
        
        return b;
    }
    
    return nullptr;
}

#define headis(a, s, rest) \
    ((a)->type() == ListType \
        && asList(a)->head->type() == SymType \
        && asList(a)->head->get_Sym() == s \
        && (rest = asList(a)->tail))
        

void define(Ptr<Env> env, Ptr<List> body);
void defineType(Ptr<Env> env, Ptr<List> body);
void defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs);

void topLevel(Ptr<Globals> globals, Ptr<Atom> exp) {
    Ptr<List> rest;
    if(headis(exp, "=", rest)) {
        define(globals, rest);
    }
}

void define(Ptr<Env> env, Ptr<List> body) {
    assert(body != nullptr);
    Ptr<Atom> lhs = body->head;
    body = body->tail;
    
    assert(body != nullptr);
    Ptr<Atom> rhs = body->head;
    assert(body->tail == nullptr);
    
    Ptr<List> head_rest;
    
    if(headis(lhs, "type", head_rest)) { // type definition
        defineType(env, head_rest);
    } else {
        defineValue(env, lhs, rhs);
    }
}

Ptr<Expression> expression(Ptr<Atom> a) {return nullptr;}
Ptr<Expression> function(Ptr<List> args) {return nullptr;}

void defineType(Ptr<Env> env, Ptr<List> body) {}

void defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs) {
    Sym name;
    Ptr<List> params;
    Ptr<List> *paramsDest;
    
    while(lhs->type() == ListType) {
        Ptr<List> rest = asList(lhs)->tail->copy();
        paramsDest = appendList(paramsDest, rest);
        lhs = asList(lhs)->head;
    }
    
    assert(lhs->type() == SymType);
    name = lhs->get_Sym();
    
    Ptr<Expression> exp = 
        params != nullptr ? function(list({params, rhs}))
                          : expression(rhs);
}
