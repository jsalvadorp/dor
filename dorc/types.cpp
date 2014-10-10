#include <cassert>
#include "types.hpp"
#include "ast.hpp"
#include "analysis.hpp"

using namespace ast;
/*
Ptr<Kind> Kind::k0() {
    static Ptr<Kind> k0 = newPtr<Kind>(nullptr, nullptr);
    return k0;
}*/

bool unifyKinds(Ptr<Kind> &left, Ptr<Kind> &right) {
    if(left == nullptr) left = right;
    if(right == nullptr) right = left;
    
    return left == right
        || (left != K1 && right != K1
            && unifyKinds(left->from, right->from)
            && unifyKinds(left->to, right->to));
}

Ptr<Kind> applyKinds(Ptr<Kind> &left, Ptr<Kind> &right) {
    Ptr<Kind> application = newPtr<Kind>(right, nullptr);
    assert(unifyKinds(application, left));
    return application->to;
}

TypeApp::TypeApp(Ptr<Type> left, Ptr<Type> right) : Type(), left(left), right(right) {
    // calculate kind
    kind = applyKinds(left->kind, right->kind);
}

Ptr<Type> applyTypes(Ptr<Type> &left, Ptr<Type> &right) {
    Ptr<Type> application = funcType(right, newPtr<TypeVar>(K1));
    assert(unifyTypes(application, left));
    return funcTypeTo(application);
}

bool unifyTypes(Ptr<Type> &x, Ptr<Type> &y) {
    /*
    if(x == nullptr) x = y;
    if(y == nullptr) y = x;
    */
    assert(x != nullptr && y != nullptr);
    
    //x = x->getRoot(), y = y->getRoot();
    shorten(x);
    shorten(y);
    
    // x debe ser de menor rango que y!!!!!!!!!!
    if(y->getRank() < x->getRank())
        return unifyTypes(y, x);
    
    if(x->type() == TFORALL) {
        Ptr<Type> x_i = 
            castPtr<TypeForAll, Type>(x)->instance();
        
        return unifyTypes(x_i, y);
    }
    
    if(y->type() == TFORALL) {
        Ptr<Type> y_i = 
            castPtr<TypeForAll, Type>(y)->instance();
        
        assert(y_i != nullptr);
        return unifyTypes(x, y_i);
    }
    
    if(x == y) return true;
    
    // TODO: check interface membership
    
    if(!unifyKinds(x->kind, y->kind)) 
        return false;
    
    
    /*if(y->isUnboundVar()) {
        assert(!x->contains(castPtr<TypeVar, Type>(y))); // occurs check
        if(x )
        y->setParent(x);
        y = x;
        return true;
    } else */if(x->isUnboundVar()) {
        assert(!y->contains(castPtr<TypeVar, Type>(x))); // occurs check
        
        // y is a bound var
        if(y->type() == TVAR && y->getRank() == RANK_INF) {
            std::cout << "unifying var ";
            x->dump();
            std::cout << " with bound var ";
            y->dump();
            std::cout << std::endl;
        }
        
        x->setParent(y);
        x = y;
        return true;
    } else if(x->type() == TAPP && y->type() == TAPP) {
        Ptr<TypeApp> 
            x_a = castPtr<TypeApp, Type>(x), 
            y_a = castPtr<TypeApp, Type>(y);
        if(unifyTypes(x_a->left, y_a->left)
            && unifyTypes(x_a->right, y_a->right)) {
                x = y;
            return true;
        } 
    } else if(x->type() != TAPP && y->type() != TAPP) {
        return x == y;
    }
     
    return false;
}


#if 0

 function MakeSet(x)
     x.parent := x
     x.rank   := 0
 
 function Union(x, y)
     xRoot := Find(x)
     yRoot := Find(y)
     if xRoot.rank > yRoot.rank
         yRoot.parent := xRoot
     else if xRoot.rank < yRoot.rank
         xRoot.parent := yRoot
     else if xRoot != yRoot
         yRoot.parent := xRoot
         xRoot.rank := xRoot.rank + 1
  
 function Find(x)
     if x.parent == x
        return x
     else
        x.parent := Find(x.parent)
        return x.parent

#endif

Ptr<Type> Int, String, Char, Bool, Void, Float, FuncArrow;


#define DEFINE_BUILTIN_TYPE(id, name, kind) \
    g->type_dictionary[name] = id = newPtr<Type>(name, kind)
void initTypes(Ptr<Globals> g) {
    DEFINE_BUILTIN_TYPE(Int, "Int", K1);
    DEFINE_BUILTIN_TYPE(Char, "Char", K1);
    DEFINE_BUILTIN_TYPE(String, "String", K1);
    DEFINE_BUILTIN_TYPE(Bool, "Bool", K1);
    DEFINE_BUILTIN_TYPE(Void, "Void", K1);
    DEFINE_BUILTIN_TYPE(Float, "Float", K1);
    DEFINE_BUILTIN_TYPE(FuncArrow, "->", K3);
    /*Int         = newPtr<Type>("Int", K1);
    Char        = newPtr<Type>("Char", K1);
    String      = newPtr<Type>("String", K1);
    Bool        = newPtr<Type>("Bool", K1);
    Void        = newPtr<Type>("Void", K1);
    Float       = newPtr<Type>("Float", K1);
    FuncArrow   = newPtr<Type>("->", K3);*/ // kind * -> * -> * !
}

Ptr<Type> funcType(Ptr<Type> from, Ptr<Type> to) {
    Ptr<Kind> k1 = K1;
    unifyKinds(from->kind, k1);
    unifyKinds(to->kind, k1);
    return newPtr<TypeApp>(newPtr<TypeApp>(FuncArrow, from), to);
}

// is it a complete application of the arrow?
bool isFuncType(Ptr<Type> ft, Ptr<Type> *from, Ptr<Type> *to) {
    //assert(ft != nullptr && ft->isApp());
    
    Ptr<Type> _from, _to;
    
    if(ft->type() != TAPP) return false;
    Ptr<TypeApp> fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    _to = fta->right;
    //assert(fta->left == FuncArrow);
    ft = fta->left;
    if(ft->type() != TAPP) return false;
    fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    _from = fta->right;
    
    if(fta->left == FuncArrow) {
        if(from != nullptr)
            *from = _from;
        if(to != nullptr)
            *to = _to;
        return true;
    }
    return false;
}

Ptr<Type> funcTypeFrom(Ptr<Type> ft) {
    //assert(ft != nullptr && ft->isApp());
    Ptr<Type> from, to;
    
    Ptr<TypeApp> fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    to = fta->right;
    //assert(fta->left == FuncArrow);
    ft = fta->left;
    fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    from = fta->right;
    assert(fta->left == FuncArrow);
    
    return from;
}

Ptr<Type> funcTypeTo(Ptr<Type> ft) {
    //assert(ft != nullptr && ft->isApp());
    Ptr<Type> from, to;
    
    Ptr<TypeApp> fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    to = fta->right;
    //assert(fta->left == FuncArrow);
    ft = fta->left;
    fta = std::dynamic_pointer_cast<TypeApp, Type>(ft);
    from = fta->right;
    assert(fta->left == FuncArrow);
    
    return to;
}

Ptr<Type> TypeForAll::instance() {
    std::vector<Ptr<TypeVar> > new_vars;
    
    for(int i = 0; i < bound_vars.size(); i++) {
        new_vars.push_back(newPtr<TypeVar>(bound_vars[i]->kind));
    }
    
    return right->substitute(castPtr<TypeForAll, Type>(shared_from_this()), new_vars);
}

Ptr<Type> TypeForAll::substitute(Ptr<TypeForAll> old_q, std::vector<Ptr<TypeVar> > &new_q) {
    shorten(right);
    
    Ptr<Type> r_subs = right->substitute(old_q, new_q);
    
    if(r_subs != right) { // have to create a new forall!
        Ptr<TypeForAll> f = newPtr<TypeForAll>();
        f->kind = kind;
        f->right = r_subs;
        
        for(int i = 0; i < bound_vars.size(); i++) {
            Ptr<TypeVar> tv = newPtr<TypeVar>(bound_vars[i]->kind);
            tv->id = i;
            tv->rank = RANK_INF;
            tv->quantifier = f.get();
            f->bound_vars.push_back(tv);
        }
        
        f->right = substitute(
            castPtr<TypeForAll, Type>(shared_from_this()),
            f->bound_vars);
            
        return f;
    }
    
    return shared_from_this(); // no substitution happened
}



void TypeForAll::dump() {
    //std::cout << (name.valid() ? name.str() : "?") << " ";
    std::cout << "(";
    for(Ptr<TypeVar> tv : bound_vars) {
        //if(tv->name.valid()) std::cout << tv->name.str() << " ";
        //else std::cout << "t" << tv->id << " ";
        tv->dump();
    }
    std::cout << "=> ";
    
    right->dump();
    
    std::cout << ") ";
}

void TypeForAll::capture(Ptr<TypeVar> variable) {
    if(variable->quantifier != nullptr) {
        // variable is already bound. check that it is bound to this quantifier
        assert(variable->quantifier == this);
        return;
    }
    
    // otherwise, bind it
    variable->quantifier = this;
    variable->id = bound_vars.size();
    variable->setRank(RANK_INF);
    variable->name = variable->name.valid() ? variable->name : Sym(std::string("t") + std::to_string(variable->id));
    bound_vars.push_back(variable);
}


// check that a given type has more constraints that another
// right know it only works if they are only quantified at the outermost
// part, no inner quantifications. And it does not consider interfaces,
// just the number of variables
bool isMoreConstrained(Ptr<Type> a, Ptr<Type> b) {
    if(a->type() == TFORALL) {
        Ptr<TypeForAll> fa = castPtr<TypeForAll, Type>(a);
        if(b->type() == TFORALL) {
            Ptr<TypeForAll> fb = castPtr<TypeForAll, Type>(b);
            return fa->bound_vars.size() <= fb->bound_vars.size(); // has less open variables
        } else return true;
    } else return true; // if the user-given type is not a forall, it is a solid type without variables of any kind
}
