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
    return args != nullptr && arity_is_min<N-1>(args->tail);
}
template<>
bool arity_is_min<0>(Ptr<List> args) {
    return true;
}

/*
 * 
Forward declarations
* 
* acceptable
* # local
* a : Int
* a = 5
* print a
* 
* # local/global
* fac n = if (n == 0) 1 (n * fac (n - 1)))
* 
* global
* f : Int -> Int
* p a = f a + 5
* 
* unacceptable
* 
* local
* a : Int
* print a
* 
* a :

*/

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
            // captured locals must be previously defined as they are
            // captured by value
            assert(b->defined); 
            
            Binding clos(*b, CLOSURE);
            // TODO consider making closure values CONST to avoid errors:
            // since they are captured by value, programmers might make
            // mistakes thinking they can mutate their values on the 
            // surrounding context
            // clos.mut = CONST;
            clos.mut = VAR;
            
            dictionary[name] = clos;
            closure.push_back(&dictionary[name]);
            return closure.back();
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
Ptr<Expression> declareValue(Ptr<Env> env, Ptr<List> args);


Ptr<Application> application(Ptr<Env> env, Ptr<List> args);
Ptr<Function> function(Ptr<Env> env, Ptr<List> args, Binding *binding = nullptr);
Ptr<Sequence> sequence(Ptr<Env> env, Ptr<List> args);
Ptr<Expression> expression(Ptr<Env> env, Ptr<Atom> a, Binding *binding = nullptr);
Ptr<Reference> reference(Ptr<Env> env, Ptr<Atom> a);

Ptr<TypeApp> typeApplication(Ptr<TypeEnv> env, Ptr<List> args);
Ptr<Type> typeExpression(Ptr<TypeEnv> env, Ptr<Atom> exp);

Ptr<Expression> topLevel(Ptr<Globals> globals, Ptr<Atom> exp) {
    Ptr<List> rest;
    if(headis(exp, "=", rest)) {
        return define(globals, rest);
    } else if(headis(exp, ":", rest)) {
        return declareValue(globals, rest);
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
        return defineValue(
            env, 
            head_rest->tail == nullptr ? head_rest->head : head_rest, 
            rhs, 
            true);
    } else {
        return defineValue(env, lhs, rhs, false);
    }
}

Ptr<Conditional> if_(Ptr<Env> env, Ptr<List> args);

Ptr<Reference> reference(Ptr<Env> env, Ptr<Atom> a) {
    Binding *b = env->find(a->get_Sym());
    
    if(!b || (!b->defined && b->scope != GLOBAL)) { // only globals can be forward-declared
        std::cerr << "Undefined " << a->get_Sym().str() << std::endl;
        assert(!"Undefined symbol");
    }
    
    return newPtr<Reference>(b);
}

Ptr<Expression> expression(Ptr<Env> env, Ptr<Atom> a, Binding *binding) {
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
        if(a->get_Sym() == Sym("True")) return newPtr<Literal>(atom(true), Bool);
        if(a->get_Sym() == Sym("False")) return newPtr<Literal>(atom(false), Bool);
        return reference(env, a); // nulll type!!!!!??
    }
    case ListType: {
        Ptr<List> a_list = asList(a);
        Ptr<List> a_rest;
        if(headis(a, "->", a_rest)) {
            return function(env, a_rest, binding);
        } else if(headis(a, "do", a_rest)) {
            return sequence(env, a_rest);
        } else if(headis(a, "if", a_rest)) {
            return if_(env, a_rest);
        } else if(headis(a, "=", a_rest)) {
            return define(env, a_rest);
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
        app = newPtr<Application>(head, expression(env, arg));
        head = app;
    }
    
    return app;
} 

Ptr<Function> function(Ptr<Env> env, Ptr<List> args, Binding * b) {
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    
    Ptr<Type> ftype = b ? b->type : newPtr<TypeVar>(K1),
        ret = ftype;
    
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
    func->type = newPtr<TypeVar>(K1);
    
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
        binding.defined = true;
        binding.type = newPtr<TypeVar>(K1);
        
        ret = applyTypes(ret, binding.type);
        
        func->dictionary[name] = binding;
        func->parameters.push_back(&func->dictionary[name]);
    }
    
    func->body = expression(func, body);
    assert(unifyTypes(func->body->type, ret));
    assert(unifyTypes(func->type, ftype));
    
    return func;
}

Ptr<Sequence> sequence(Ptr<Env> env, Ptr<List> body) {
    assert(arity_is_min<1>(body));
    Ptr<Sequence> exp = newPtr<Sequence>(env);
    
    for(Ptr<Atom> a : *body) {
        exp->steps.push_back(expression(exp->locals, a));
    }
    
    //assert(unifyTypes(exp->steps.back()->type, exp->type));
    exp->type = exp->steps.back()->type;
    
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



Ptr<Expression> declareValue(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is<2>(args));
    
    Ptr<Type> type = typeExpression(env, args->at(1));
    
    Binding *b;
    if(args->head->type() == SymType) {
        if((b = env->find(args->head->get_Sym()))) {
            assert(unifyTypes(b->type, type));
            return reference(env, args->head);
        } else {
            b = &env->dictionary[args->head->get_Sym()];
            b->scope = env->isGlobal() ? GLOBAL : LOCAL;
           
            if(b->type == nullptr) b->type = type;
            else assert(unifyTypes(b->type, type));
            
            if(b->defined) return newPtr<Reference>(b);
            else return nullptr;
        }
    } else {
        Ptr<Expression> exp = expression(env, args->head);
        assert(unifyTypes(exp->type, type));
        return exp;
    }
}

Ptr<Assignment> defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs, bool variable) {
    Sym name;
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    
    while(lhs->type() == ListType) {
        Ptr<List> rest = asList(lhs)->tail->copy();
        params_dest = appendList(params_dest, rest);
        lhs = asList(lhs)->head;
    }
    
    assert(lhs->type() == SymType);
    name = lhs->get_Sym();
    
    // redefinitions not allowed!
    auto it = env->dictionary.find(name);
    assert(it == env->dictionary.end() || !it->second.defined);
    
    // the early binding is necessary for recursion
    Binding *new_binding = &env->dictionary[name];
    new_binding->scope = env->isGlobal() ? GLOBAL : LOCAL;
    new_binding->mut = variable ? VAR : CONST;
    new_binding->name = name;
    new_binding->type = 
        new_binding->type == nullptr    ? newPtr<TypeVar>(K1) 
                                        : new_binding->type;
    new_binding->defined = true; 
    
    Ptr<Expression> exp = 
        params != nullptr ? function(env, list({params, rhs}), new_binding)
                          : expression(env, rhs, new_binding);
    
    shorten(new_binding->type);
    
    
    
    Ptr<Assignment> ass = newPtr<Assignment>(newPtr<Reference>(new_binding), exp);
    
    Ptr<TypeForAll> q = newPtr<TypeForAll>();
    
    q->init(new_binding->type);
    
    if(!q->bound_vars.empty()) { // any free variables captured??
        assert(q->bound_vars[0]->getRoot() != Int);
        new_binding->type = q; 
    }
    
    return ass;
}


Ptr<Type> typeExpression(Ptr<TypeEnv> env, Ptr<Atom> exp) {
    if(exp->type() == SymType) {
        Ptr<Type> t =  env->findType(exp->get_Sym());
        assert(t != nullptr);
        
        return t;
    } else {
        assert(exp->type() == ListType);
        
        return typeApplication(env, asList(exp));
    }
}

Ptr<TypeApp> typeApplication(Ptr<TypeEnv> env, Ptr<List> args) {
    assert(arity_is_min<2>(args));
    
    Ptr<Type> head = typeExpression(env, args->head);
    Ptr<TypeApp> app;
    
    for(Ptr<Atom> arg : *args->tail) {
        app = newPtr<TypeApp>(head, typeExpression(env, arg));
        head = app;
    }
    
    return app;
} 
