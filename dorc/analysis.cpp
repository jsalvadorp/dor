#include <unordered_map>
#include "analysis.hpp"

using namespace ast;

// on lookup Sym("_") return null!!

template<size_t N>
bool arity_is(Ptr<List> args) {
    return args != nullptr && arity_is<N-1>(args->tail);
}
template<>
bool arity_is<0>(Ptr<List> args) {
    return args == nullptr;
}

template<size_t N>
bool arity_is_min(Ptr<List> args) {
    return args != nullptr && arity_is<N-1>(args->tail);
}
template<>
bool arity_is_min<0>(Ptr<List> args) {
    return true;
}

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
            clos.mut = VAR;
            
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
        

Ptr<Assignment> define(Ptr<Env> env, Ptr<List> body);
void defineType(Ptr<Env> env, Ptr<List> body);
Ptr<Assignment> defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs, bool variable);


Ptr<Application> application(Ptr<Env> env, Ptr<List> args);
Ptr<Function> function(Ptr<Env> env, Ptr<List> args);
Ptr<Sequence> sequence(Ptr<Env> env, Ptr<List> args);

Ptr<Expression> topLevel(Ptr<Globals> globals, Ptr<Atom> exp) {
    Ptr<List> rest;
    if(headis(exp, "=", rest)) {
        return define(globals, rest);
    } else if(headis(exp, "infixr", rest)) {
        // ignore
    } else if(headis(exp, "infixl", rest)) {
        // ignore
    }
    
    return nullptr;
}

Ptr<Assignment> define(Ptr<Env> env, Ptr<List> body) {
    assert(body != nullptr);
    Ptr<Atom> lhs = body->head;
    body = body->tail;
    
    assert(body != nullptr);
    Ptr<Atom> rhs = body->head;
    assert(body->tail == nullptr);
    
    Ptr<List> head_rest;
    
    if(headis(lhs, "type", head_rest)) { // type definition
        defineType(env, head_rest);
        return nullptr;
    } else if(headis(lhs, "var", head_rest)) { // type definition
        return defineValue(env, lhs, rhs, true);
    } else {
        return defineValue(env, lhs, rhs, false);
    }
}

Ptr<Conditional> if_(Ptr<Env> env, Ptr<List> args);

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
        } else if(headis(a, "do", a_rest)) {
            return sequence(env, a_rest);
        } else if(headis(a, "if", a_rest)) {
            return if_(env, a_rest);
        } else {
            return application(env, a_list);
        }
    }
    }
}

Ptr<Conditional> if_(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is<3>(args));
    
    return newPtr<Conditional>(
        expression(env, args->at(0)), 
        expression(env, args->at(1)), 
        expression(env, args->at(2)));
}

Ptr<Application> application(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is_min<2>(args));
    
    Ptr<Expression> head = expression(env, args->head);
    Ptr<Application> app;
    
    for(Ptr<Atom> arg : *args->tail) {
        Ptr<Application> app = newPtr<Application>(head, expression(env, arg));
        head = app;
    }
    
    return app;
} 

Ptr<Function> function(Ptr<Env> env, Ptr<List> args) {
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    
    Ptr<Atom> body;
    
    do {
        assert(arity_is<2>(args));
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
    
    Ptr<Function> func = newPtr<Function>(env);
    
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

Ptr<Sequence> sequence(Ptr<Env> env, Ptr<List> body) {
    Ptr<Sequence> exp = newPtr<Sequence>(env);
    
    for(Ptr<Atom> a : *body) {
        exp->steps.push_back(expression(env, a));
    }
    
    return exp;
}

Ptr<Sequence> program(Ptr<Globals> env, Ptr<List> body) {
    Ptr<Sequence> exp = newPtr<Sequence>();
    exp->locals = env; // global "locals"
    
    for(Ptr<Atom> a : *body) {
        //exp->steps.push_back(expression(env, a));
        Ptr<Expression> line = topLevel(env, a);
        if(line != nullptr) exp->steps.push_back(line);
    }
    
    return exp;
}

void defineType(Ptr<Env> env, Ptr<List> body) {}

Ptr<Assignment> defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs, bool variable) {
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
    
    Binding binding;
    binding.scope = env->isGlobal() ? GLOBAL : LOCAL;
    binding.mut = variable ? VAR : CONST;
    binding.name = name;
    binding.value = exp;
    
    env->dictionary[name] = binding;
    
    return newPtr<Assignment>(newPtr<Reference>(env->find(name)), exp);
}


