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
void defineType(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs);
Ptr<Assignment> defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs, bool variable);
Ptr<Expression> declareValue(Ptr<Env> env, Ptr<List> args);


Ptr<Application> application(Ptr<Env> env, Ptr<List> args);
Ptr<Function> function(Ptr<Env> env, Ptr<List> args, Ptr<Type> ftype = newPtr<TypeVar>(K1));
Ptr<Sequence> sequence(Ptr<Env> env, Ptr<List> args);
Ptr<Expression> expression(Ptr<Env> env, Ptr<Atom> a, Binding *binding = nullptr);
Ptr<Reference> reference(Ptr<Env> env, Ptr<Atom> a);
Ptr<Expression> cond(Ptr<Env> env, Ptr<List> args);

Ptr<TypeApp> typeApplication(Ptr<TypeEnv> env, Ptr<List> args);
Ptr<Type> typeExpression(Ptr<TypeEnv> env, Ptr<Atom> exp);

void defineADT(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs);

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
    } else if(exp->type() == SymType && exp->get_Sym() == Sym("break")) {
        std::cout << "break"<< std::endl;
    }
    
    return nullptr;
}

Ptr<Assignment> define(Ptr<Env> env, Ptr<List> body) {
    assert(arity_is<2>(body));
    
    Ptr<Atom> lhs = body->at(0), rhs = body->at(1);
    
    Ptr<List> head_rest;
    
    if(headis(lhs, "type", head_rest)) { // type alias definition
        // assert(env->isGlobal());
        defineType(
            env, 
            head_rest->tail == nullptr ? head_rest->head : head_rest,
            rhs
        );
        return nullptr;
    } else if(headis(lhs, "data", head_rest)) { // type alias definition
        // assert(env->isGlobal());
        defineADT(
            env, 
            head_rest->tail == nullptr ? head_rest->head : head_rest,
            rhs
        );
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
    
    // only externs and globals can be forward-declared
    if(!b || (!b->defined && b->scope != GLOBAL && b->scope != EXTERN)) { 
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
            return function(env, a_rest);
        } else if(headis(a, "do", a_rest)) {
            return sequence(env, a_rest);
        } else if(headis(a, "if", a_rest)) {
            return if_(env, a_rest);
        } else if(headis(a, "cond", a_rest)) {
            return cond(env, a_rest);
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



Ptr<Expression> cond_rec(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is_min<1>(args));
    
    Ptr<List> clause;
    
    assert(headis(args->head, "=>", clause));
    
    assert(arity_is<2>(clause));
    Ptr<Atom> condition = clause->at(0), on_true = clause->at(1);
    
    if(args->tail == nullptr) { // last clause, else clause
        assert(condition->type() == SymType 
            && condition->get_Sym() == Sym("else"));
        
        return expression(env, on_true);
    } else assert(condition->type() != SymType 
            || condition->get_Sym() != Sym("else"));
    
    return newPtr<Conditional>(
        expression(env, condition), 
        expression(env, on_true), 
        cond_rec(env, args->tail));
}


Ptr<Expression> cond(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is_min<2>(args));
    
    return cond_rec(env, args);
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

Ptr<Function> function(Ptr<Env> env, Ptr<List> args, Ptr<Type> ftype) {
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    
    //Ptr<Type> ftype = newPtr<TypeVar>(K1),
    //    ret = ftype;
    Ptr<Type> ret = ftype;
    
    Ptr<Atom> body;
    
    do {
        assert(arity_is<2>(args));
        Ptr<Atom> left = args->head;
        body = args->tail->head;
        
        if(left->type() == ListType) {
            Ptr<List> param_rest;
            if(headis(left, ":", param_rest)) {
                params_dest = append(params_dest, left);
            } else for(Ptr<Atom> p : *asList(left)) {
                params_dest = append(params_dest, p);
            }
        } else params_dest = append(params_dest, left);
    } while(headis(body, "->", args));
    
    Ptr<Function> func = newPtr<Function>(env);
    func->type = ftype;//newPtr<TypeVar>(K1);
    
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
        //binding.type = newPtr<TypeVar>(K1);
        
        // ret = applyTypes(ret, binding.type);
        
        Ptr<Type> from, to;
        shorten(ret);
        
        if(isFuncType(ret, &from, &to)) {
            
            binding.type = from;
            ret = to;
        } else {
            assert(ret->type() != TFORALL); // NO foralls on the right
            
            binding.type = newPtr<TypeVar>(K1);
            ret = applyTypes(ret, binding.type);
        }
        
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




Ptr<Expression> declareValue(Ptr<Env> env, Ptr<List> args) {
    assert(arity_is<2>(args));
    
    Ptr<Type> type = typeExpression(env, args->at(1));
    
    Binding *b;
    if(args->head->type() == SymType) {
        if((b = env->find(args->head->get_Sym()))) {
            //assert(!"tried to redeclare a value");
            assert(unifyTypes(b->type, type));
            return reference(env, args->head);
        } else {
            // new value!
            b = &env->dictionary[args->head->get_Sym()];
            b->scope = env->isGlobal() ? GLOBAL : LOCAL;
            b->name = args->head->get_Sym();
            b->defined = false;
           
            /*if(b->type == nullptr)*/ b->type = type;
            /*else assert(unifyTypes(b->type, type));*/
            
            /*if(b->defined)*/ return newPtr<Reference>(b);
            /*else */return nullptr;
        }
    } else {
        Ptr<Expression> exp = expression(env, args->head);
        assert(unifyTypes(exp->type, type));
        return exp;
    }
}


// define type alias
void defineType(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs) {
    assert(lhs->type() == SymType);
    Sym name = lhs->get_Sym();
    
    Ptr<Type> type = typeExpression(env, rhs);
    
    assert(env->type_dictionary.find(name) == env->type_dictionary.end());
    
    env->type_dictionary[name] = type;
}



Sym processLhs(Ptr<Atom> lhs, Ptr<List> *&params_dest, Ptr<Atom> *type_exp, bool can_type_head) {
    if(lhs->type() == SymType) {
        return lhs->get_Sym();
    } else {
        Ptr<List> head_rest;
        if(headis(lhs, ":", head_rest)) {
            assert(can_type_head);
            assert(arity_is<2>(head_rest));
            
            *type_exp = head_rest->at(1);
            return processLhs(head_rest->at(0), params_dest, type_exp, false);
        } else {
            Ptr<List> l = asList(lhs);
            assert(arity_is_min<2>(l));
            
            Sym name = processLhs(l->head, params_dest, type_exp, false);
            
            for(Ptr<Atom> arg : *l->tail) {
                params_dest = append(params_dest, arg);
            }
            
            return name;
        }
    }
}


void typeQuantifiedLeft(Ptr<TypeEnv> new_env, Ptr<TypeForAll> tfa, Ptr<Atom> left) {
    // handle the colon! for interface constraints
    
    if(left->type() == SymType) {
        Sym name = left->get_Sym();
        Ptr<TypeVar> v = newPtr<TypeVar>(name);
        
        assert(
            new_env->type_dictionary.find(name) == 
            new_env->type_dictionary.end());
        tfa->capture(v);
        new_env->type_dictionary[name] = v;
    } else {
        assert(left->type() == ListType);
        for(Ptr<Atom> a : *asList(left)) {
            assert(a->type() == SymType); // TODO: handle interface constraints
            Sym name = a->get_Sym();
            Ptr<TypeVar> v = newPtr<TypeVar>(name);
            
            assert(
                new_env->type_dictionary.find(name) == 
                new_env->type_dictionary.end());
            tfa->capture(v);
            new_env->type_dictionary[name] = v;
        }
    }
}

#if 0



Ptr<TypeForAll> typeQuantified(Ptr<TypeEnv> env, Ptr<List> args) {
    assert(arity_is<2>(args));
    
    Ptr<Atom> left = args->at(0), right = args->at(1);
    
    Ptr<TypeForAll> tfa = newPtr<TypeForAll>();
    Ptr<TypeEnv> new_env = newPtr<TypeEnv>(env);
    
    typeQuantifiedLeft(new_env, tfa, left)
    
    tfa->right = typeExpression(new_env, right);
    
    return tfa;
}

#endif


Binding *constructor(Ptr<Env> env, Ptr<Atom> con, Ptr<Type> new_type, Ptr<List> type_params) {
    Ptr<Atom> quant_left; // any extra quantification for this constructor
    
    Ptr<List> con_rest;
    if(headis(con, "=>", con_rest)) {
        assert(arity_is<2>(con_rest));
        // assert(!"Existentially quantified constructor not available ATM");
        quant_left = con_rest->at(0);
        con = con_rest->at(1);
    }
    
    // quantification (in case there are type parameters or constructor 
    // (existential) quantification)
    
    Ptr<TypeForAll> tfa = newPtr<TypeForAll>();
    Ptr<TypeEnv> new_env = newPtr<TypeEnv>(env);
    
    if(type_params != nullptr) 
        typeQuantifiedLeft(new_env, tfa, type_params);
    // which variables in the quantification appear in the return type?
    size_t parametric = tfa->bound_vars.size();
    if(quant_left != nullptr) 
        typeQuantifiedLeft(new_env, tfa, quant_left);
    
    // build the types
    Ptr<Type> ctype = newPtr<TypeVar>(K1), ret = ctype;
    
    Sym name;
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    Ptr<Atom> type_exp;
    
    name = processLhs(con, params_dest, &type_exp, true);
    
    if(params != nullptr) for(Ptr<Atom> a : *params) {
        Ptr<Type> ptype;
        Ptr<List> a_rest;
        
        if(headis(a, ":", a_rest)) {
            assert(arity_is<2>(a_rest));
            
            // a_rest->at(0) is the argument (member) name
            
            ptype = typeExpression(new_env, a_rest->at(1));
        } else {
            ptype = typeExpression(new_env, a);
        }
        
        ret = applyTypes(ret, ptype);
    }
    
    shorten(ctype);
    shorten(ret);
    
    if(type_exp != nullptr) { // GADT
        Ptr<Type> rtype = typeExpression(new_env, type_exp);
        
        // TODO: check that rtype is a complete application of new_type
        assert(!"GADTs not implemented");
    } else {
        Ptr<Type> matcher = new_type;
        
        for(size_t i = 0; i < parametric; i++) {
            matcher = newPtr<TypeApp>(matcher, tfa->bound_vars[i]);
        }
        
        assert(unifyTypes(ret, matcher));
    }
    
    if(tfa->bound_vars.size()) {
        tfa->right = ctype;
        ctype = tfa;
    }
    
    assert(env->isGlobal());
    assert(env->dictionary.find(name) == env->dictionary.end());
    Binding *new_binding = &env->dictionary[name];
    new_binding->scope = env->isGlobal() ? GLOBAL : LOCAL;
    new_binding->mut = CONST;
    new_binding->name = name;
    new_binding->type = ctype;    
    new_binding->defined = true; 
    new_binding->constructor = true;
    
    return new_binding;
}

void defineADT(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs) {
    Sym name;
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    Ptr<Atom> type_exp;
    
    name = processLhs(lhs, params_dest, &type_exp, false);
    
    assert(env->type_dictionary.find(name) == env->type_dictionary.end());
    
    Ptr<AlgebraicType> newt;
    env->type_dictionary[name] = newt = 
        newPtr<AlgebraicType>(name, params == nullptr ? K1 : nullptr); // if params, then kind is still unknown
    
    Ptr<List> constructors;
    
    if(!headis(rhs, "|", constructors))
        constructors = newPtr<List>(rhs, nullptr);
    
    for(Ptr<Atom> con : *constructors) {
        newt->constructors.push_back(constructor(env, con, newt, params));
    }
}
    
   /* 
    // for now: declarations of the form    f (a:Int) (b:Int) :Int = ...
    // are NOT allowed
    assert(params == nullptr || type_exp == nullptr);
    
    if(lhs->type() == SymType)
        name = lhs->get_Sym();
    else {
        assert(lhs->type() == ListType);
        assert(asList(lhs)->head->type() == SymType);
        name = asList(lhs)->head->get_Sym();
        
        type_params = asList(lhs)->tail;
    }
    
    assert(lhs->type() == SymType);
    Sym name = lhs->get_Sym();
    
    Ptr<Type> type = typeExpression(env, rhs);
    
    assert(env->type_dictionary.find(name) == env->type_dictionary.end());
    
    env->type_dictionary[name] = type;
}*/

Ptr<Assignment> defineValue(Ptr<Env> env, Ptr<Atom> lhs, Ptr<Atom> rhs, bool variable) {
    Sym name;
    Ptr<List> params;
    Ptr<List> *params_dest = &params;
    Ptr<Atom> type_exp;
    
    name = processLhs(lhs, params_dest, &type_exp, true);
    
    // for now: declarations of the form    f (a:Int) (b:Int) :Int = ...
    // are NOT allowed
    assert(params == nullptr || type_exp == nullptr);
    
    // redefinitions not allowed!
    auto it = env->dictionary.find(name);
    assert(it == env->dictionary.end() 
        || (it->second.scope != EXTERN && !it->second.defined));
    
    bool had_a_type = false; // had a type before defining
    
    // the early binding is necessary for recursion
    Binding *new_binding = &env->dictionary[name];
    new_binding->scope = env->isGlobal() ? GLOBAL : LOCAL;
    new_binding->mut = variable ? VAR : CONST;
    new_binding->name = name;
    
    // had_a_type means user provided a complete type without free variables
    had_a_type = new_binding->type != nullptr;
    if(!had_a_type) // else give it a typevar so recursion can aid us in finding a type
        new_binding->type = newPtr<TypeVar>(K1);
        
        
    new_binding->defined = true; 
    
    Ptr<Expression> exp;    
    
    if(had_a_type) { // let's match the type of exp with user-type
        if(new_binding->type->type() == TFORALL) {
            // we must unify with ONLY the right part of the quantified
            // type. The body of the function/rhs is considered to be
            // in the context of the quantifier: all variables must be
            // "rigid" or quantified to it. if we didn't, the unifier 
            // would make an instance of new_binding->type and that 
            // would make some freevars
            Ptr<TypeForAll> tfa = castPtr<TypeForAll>(new_binding->type);
            //assert(unifyTypes(tfa->right, exp->type));
            //shorten(exp->type);
            
            if(params != nullptr) {
                exp = function(env, list({params, rhs}), tfa->right);
                assert(unifyTypes(tfa->right, exp->type));
                shorten(exp->type);
            } else {
                exp = expression(env, rhs, new_binding);
                assert(unifyTypes(tfa->right, exp->type));
                shorten(exp->type);
            }
        } else { // assuming no more quantifiers inside new_binding->type!!
            exp = params != nullptr 
                ? function(env, list({params, rhs}))
                : expression(env, rhs, new_binding);  
            assert(unifyTypes(new_binding->type, exp->type));
            shorten(exp->type);
        }
    } else { // no user-given type. capture freevars!
        exp = params != nullptr 
                ? function(env, list({params, rhs}))
                : expression(env, rhs, new_binding);  
                          
        Ptr<TypeForAll> q = newPtr<TypeForAll>();
        q->init(exp->type);
        if(!q->bound_vars.empty()) { // any free variables captured??
            new_binding->type = q; 
        } else new_binding->type = exp->type;
    }
    
    Ptr<Reference> lref = newPtr<Reference>(new_binding);
    // unification already happened so the following must be ok
    Ptr<Assignment> ass = newPtr<Assignment>(lref, exp);
    return ass;
}

Ptr<TypeForAll> typeQuantified(Ptr<TypeEnv> env, Ptr<List> args);

Ptr<Type> typeExpression(Ptr<TypeEnv> env, Ptr<Atom> exp) {
    if(exp->type() == SymType) {
        Ptr<Type> t =  env->findType(exp->get_Sym());
        assert(t != nullptr);
        
        return t;
    } else {
        assert(exp->type() == ListType);
        
        Ptr<List> exp_rest;
        if(headis(exp, "=>", exp_rest))
            return typeQuantified(env, exp_rest);
        
        return typeApplication(env, asList(exp));
    }
}

// disallow nested quantifiers on the right side of a ->.
// that is, user cannot write "a => a -> Int -> (b => b -> Bool)";
// must write "a b => a -> Int -> b -> Bool"
Ptr<TypeForAll> typeQuantified(Ptr<TypeEnv> env, Ptr<List> args) {
    assert(arity_is<2>(args));
    
    Ptr<Atom> left = args->at(0), right = args->at(1);
    
    Ptr<TypeForAll> tfa = newPtr<TypeForAll>();
    Ptr<TypeEnv> new_env = newPtr<TypeEnv>(env);
    
    typeQuantifiedLeft(new_env, tfa, left);
    
    tfa->right = typeExpression(new_env, right);
    
    return tfa;
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

void initExtern(Ptr<Globals> g, Sym name, Ptr<Type> type) {
    Binding *new_binding = &g->dictionary[name];
    new_binding->scope = EXTERN;
    new_binding->mut = CONST;
    new_binding->name = name;
    new_binding->defined = false;
    new_binding->type = type;
}

void initGlobals(Ptr<Globals> g) {
    Ptr<Type> 
        intIntInt = funcType(Int, funcType(Int, Int)),
        intIntBool = funcType(Int, funcType(Int, Bool));
        
    initExtern(g, Sym("+"), intIntInt);
    initExtern(g, Sym("-"), intIntInt);
    initExtern(g, Sym("*"), intIntInt);
    initExtern(g, Sym("/"), intIntInt);
    initExtern(g, Sym("%"), intIntInt);
    initExtern(g, Sym("<"), intIntBool);
    initExtern(g, Sym(">"), intIntBool);
    initExtern(g, Sym("<="), intIntBool);
    initExtern(g, Sym(">="), intIntBool);
    initExtern(g, Sym("=="), intIntBool);
    initExtern(g, Sym("!="), intIntBool);
}
