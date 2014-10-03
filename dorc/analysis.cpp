#include <unordered_map>
#include "analysis.hpp"

using namespace ast;

// on lookup Sym("_") return null!!

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
        if(b->scope == PARAMETER || b->scope == LOCAL || b->scope == CLOSURE) {
            Binding clos(*b, CLOSURE);
            // TODO consider making closure values CONST to avoid errors:
            // since they are captured by value, programmers might make
            // mistakes thinking they can mutate their values on the 
            // surrounding context
            // clos.mut = CONST;
            
            dictionary[name] = clos;
            closure.push_back(&dictionary[name]);
        }
        
        return b;
    }
    
    return nullptr;
}

#define headis(a, s, rest) \
    ((a)->type() == ListType \
        && asList(a)->head->type() == SymType \
        && asList(a)->head->get_Sym() == s \
        && (rest = asList(a)->tail, true))
        

void define(Ptr<Env> env, Ptr<List> body);
void defineType(Ptr<Env> env, Ptr<List> body);
void defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs);


Ptr<Expression> application(Ptr<Env> env, Ptr<List> args);
Ptr<Expression> function(Ptr<Env> env, Ptr<List> args);

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



Ptr<Expression> expression(Ptr<Env> env, Ptr<Atom> a) {
    switch(a->type()) {
    case I64Type: {
        return newPtr<Literal>(a, Int);
    }
    case StringType: {
        return newPtr<Literal>(a, String);
    }
    case CharType: {
        return newPtr<Literal>(a, Char);
    }
    case DoubleType: {
        return newPtr<Literal>(a, Float);
    }
    case BoolType: {
        return newPtr<Literal>(a, Bool);
    }
    case VoidType: {
        return newPtr<Literal>(a, Void);
    }
    case SymType: {
        return newPtr<Reference>(env->find(a->get_Sym())); // nulll type!!!!!??
    }
    case ListType: {
        Ptr<List> a_list = asList(a);
        Ptr<List> a_rest;
        if(headis(a, "->", a_rest)) {
            return function(env, a_rest);
        } else {
            return application(env, a_list);
        }
    }
    }
}

Ptr<Expression> application(Ptr<Env> env, Ptr<List> args) {
    assert(args != nullptr && args->tail != nullptr);
    
    Ptr<Expression> head = expression(env, args->head);
    
    for(Ptr<Atom> arg : *args->tail) {
        Ptr<Application> app = newPtr<Application>(head, expression(env, arg));
        head = app;
    }
    
    return head;
} 

Ptr<Expression> function(Ptr<Env> env, Ptr<List> args) {
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    
    Ptr<Atom> body;
    
    do {
        assert(args != nullptr 
            && args->tail != nullptr 
            && args->tail->tail == nullptr);
        Ptr<Atom> left = args->head;
        body = args->tail->head;
        
        if(left->type() == ListType) {
            Ptr<List> param_rest;
            if(headis(left, ":", param_rest))
                params_dest = append(params_dest, left);
            else for(Ptr<Atom> p : *asList(left))
                params_dest = append(params_dest, p);
        } else params_dest = append(params_dest, left);
    } while(headis(body, "->", args));
    
    Ptr<Function> func = newPtr<Function>();
    
    for(Ptr<Atom> p : *params) {
        Sym name;
        Ptr<List> p_rest;
        
        if(headis(p, ":", p_rest)) {
            assert(asList(p)->head->type() == SymType);
            name = asList(p)->head->get_Sym();
            
           // TODO handle type!
        } else {
            assert(p->type() == SymType);
            name = p->get_Sym();
        }
        
        Binding binding;
        binding.scope = PARAMETER;
        binding.mut = VAR;
        binding.name = name;
        
        func->dictionary[name] = binding;
        func->parameters.push_back(&func->dictionary[name]);
    }
    
    func->body = expression(env, body);
    
    return func;
}

void defineType(Ptr<Env> env, Ptr<List> body) {}

void defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs) {
    Sym name;
    Ptr<List> params;
    Ptr<List> *params_dest;
    
    while(lhs->type() == ListType) {
        Ptr<List> rest = asList(lhs)->tail->copy();
        params_dest = appendList(params_dest, rest);
        lhs = asList(lhs)->head;
    }
    
    assert(lhs->type() == SymType);
    name = lhs->get_Sym();
    
    Ptr<Expression> exp = 
        params != nullptr ? function(env, list({params, rhs}))
                          : expression(env, rhs);
}
