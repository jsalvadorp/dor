#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "ast.hpp"
#include "symbol.hpp"

using ast::Ptr;
using ast::WeakPtr;

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

struct Globals;
void initTypes(Ptr<Globals> g);

#define INFINITY 1000000000 // the rank of bound types

enum TypeType {
    TYPE,
    TFORALL,
    TVAR,
    TAPP
};

struct TypeForAll;
struct TypeVar;

struct Type : std::enable_shared_from_this<Type> {
    Ptr<Kind> kind;
    Sym name;
    
    Type(Ptr<Kind> kind = nullptr) : kind(kind), name(-1) {}
    Type(Sym name, Ptr<Kind> kind = nullptr) : name(name), kind(kind) {}
    Type(const char *s, Ptr<Kind> kind = nullptr) : name(Sym(s)), kind(kind) {}
    
    // too many virtual functions
    
    virtual Ptr<Type> getRoot() {return shared_from_this();}
    virtual size_t getRank() {return INFINITY;}
    virtual void setRank(size_t s) {}
    virtual void setParent(Ptr<Type> t) {assert(!"Tried to reparent bound type");}
    
    virtual TypeType type() {return TYPE;}
    
    virtual bool isUnboundVar() {return false;}
    
    virtual void dump() {
        std::cout << (name.valid() ? name.str() : "?") << " ";
    }
    
    virtual void quantify(Ptr<TypeForAll> quantifier) {
        // nothing
    }
    
    virtual Ptr<Type> substitute(Ptr<TypeForAll> old_q, std::vector<Ptr<TypeVar> > &new_q) {
        return shared_from_this();
    }
    
    virtual bool contains(Ptr<TypeVar> v) {return false;}
};

inline Ptr<Type> &shorten(Ptr<Type> &t) {
    return t = t->getRoot();
}

struct TypeForAll : Type {
    Ptr<Type> right;
    std::vector<Ptr<TypeVar> > bound_vars;
    
    virtual TypeType type() {return TFORALL;}
    TypeForAll() : Type() {}
    
    void init(Ptr<Type> type) {
        right = type;
        
        right->quantify(std::dynamic_pointer_cast<TypeForAll, Type>(shared_from_this()));
        shorten(right);
        
        kind = right->kind;
    }
    
    
    void capture(Ptr<TypeVar> variable);
    Ptr<Type> instance();
    
    virtual void dump();
    
    // do nothing on quantification!
    virtual void quantify(Ptr<TypeForAll> quantifier) {
        // nothing
    }
    virtual Ptr<Type> substitute(Ptr<TypeForAll> old_q, std::vector<Ptr<TypeVar> > &new_q);
//private:
    //std::unordered_set<Ptr<TypeVar> > bound_vars;
    virtual bool contains(Ptr<TypeVar> v) {
        // should it assert against bound_vars?
        return right->contains(v);
    }
};

bool isFuncType(Ptr<Type> ft, Ptr<Type> *from = nullptr, Ptr<Type> *to = nullptr);

struct TypeApp : Type {
    Ptr<Type> left;
    Ptr<Type> right;
    
    virtual TypeType type() {return TAPP;}
    
    TypeApp() : Type() {}
    TypeApp(Ptr<Type> left, Ptr<Type> right);
    
    virtual bool isApp() {return true;}
    virtual bool isVar() {return false;}
    virtual bool isUnboundVar() {return false;}
    
    virtual void dump() {
        Ptr<Type> from, to;
        if(isFuncType(shared_from_this(), &from, &to)) {
            std::cout << "(";
            from->dump();
            std::cout << "-> ";
            to->dump();
            std::cout << ") ";
            return;
        }
        
        std::cout << "("; 
        left->dump();
        right->dump();
        std::cout << ")" << " ";
    }
    
    virtual void quantify(Ptr<TypeForAll> quantifier) {
        left->quantify(quantifier);
        right->quantify(quantifier);
        
        shorten(left);
        shorten(right);
    }
    
     virtual Ptr<Type> substitute(Ptr<TypeForAll> old_q, std::vector<Ptr<TypeVar> > &new_q) {
        shorten(left);
        shorten(right);
        
        Ptr<Type> 
            l_subs = left->substitute(old_q, new_q), 
            r_subs = right->substitute(old_q, new_q);
        
        if(left != l_subs || right != r_subs) {
            Ptr<TypeApp> app = newPtr<TypeApp>();
            app->kind = kind;
            app->left = l_subs;
            app->right = r_subs;
            return app;
        }
        
        return shared_from_this();
    }
    
    virtual bool contains(Ptr<TypeVar> v) {
        // should it assert against bound_vars?
        return left->contains(v) || right->contains(v);
    }
};



struct TypeVar : Type {
    Ptr<Type> parent;
    size_t rank; // if rank == INFINITY then variable is bound
    
    virtual TypeType type() {return TVAR;}
    
    TypeForAll *quantifier; // non-owning pointer (circular reference)
    int id;
    
    TypeVar(Ptr<Kind> kind = nullptr) 
        : Type(kind), rank(0), quantifier(nullptr) {}
    TypeVar(Sym name, Ptr<Kind> kind = nullptr) 
        : Type(name, kind), rank(0), quantifier(nullptr) {}
    
    virtual Ptr<Type> getRoot() {
        if(parent == nullptr)
            return shared_from_this();
        else {
            return parent = parent->getRoot();
        }
    }
    
    virtual size_t getRank() {
        return rank;
    }
    
    virtual void setRank(size_t rank) {
        this->rank = rank;
    }
    
    virtual void setParent(Ptr<Type> p) {
        if(p.get() == this)
            parent = nullptr; // avoid circular loops
        else
            parent = p;
    }
    
    virtual bool isApp() {return false;}
    virtual bool isUnboundVar() {
        Ptr<Type> root = getRoot();
        return root->getRank() < INFINITY;
    }
    virtual bool isVar() {return true;}
    
    virtual void quantify(Ptr<TypeForAll> q) {
        if(parent == nullptr) {
            q->capture(std::dynamic_pointer_cast<TypeVar, Type>(shared_from_this()));
        } else {
            getRoot()->quantify(q);
        }
    }
    
    virtual void dump() {
        if(parent != nullptr) getRoot()->dump();
        else if(name.valid()) std::cout << name.str() << " ";
        else if(quantifier != nullptr) std::cout << "t" << id << " ";
        else std::cout << "? ";
    }
    
    virtual Ptr<Type> substitute(Ptr<TypeForAll> old_q, std::vector<Ptr<TypeVar> > &new_q) {
        assert(!isUnboundVar()); // attempting to substitute on an expression with free variables!

        if(parent == nullptr) {
            if(quantifier == old_q.get()) { // substitute!
                return new_q[id];
            } else { // quantified over something else
                return shared_from_this();
            }
        }

        Ptr<Type> root = getRoot();

        return root->substitute(old_q, new_q);
    }
    
    virtual bool contains(Ptr<TypeVar> v) {
        
        return parent == nullptr
            ? v->getRoot().get() == this
            : getRoot()->contains(v);
    }
};

bool unifyTypes(Ptr<Type> &x, Ptr<Type> &y);
Ptr<Type> funcType(Ptr<Type> from, Ptr<Type> to);
Ptr<Type> funcTypeFrom(Ptr<Type> ft);
Ptr<Type> funcTypeTo(Ptr<Type> ft);

Ptr<Type> applyTypes(Ptr<Type> &left, Ptr<Type> &right);

struct TypeEnv {
    Ptr<TypeEnv> parent;
    std::unordered_map<Sym, Ptr<Type> > type_dictionary;
    TypeEnv(Ptr<TypeEnv> parent = nullptr) : parent(parent) {}
    
    Ptr<Type> findType(Sym s) {
        auto it = type_dictionary.find(s);
        if(it != type_dictionary.end()) {
            return it->second;
        } else if(parent != nullptr) {
            return parent->findType(s);
        } else return nullptr;
    }
};



extern Ptr<Type> Int, String, Char, Bool, Void, Float, FuncArrow;
