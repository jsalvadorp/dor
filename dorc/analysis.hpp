#include <unordered_map>
#include "ast.hpp"

using ast::Ptr;

typedef enum {
    EXTERN,
    GLOBAL, 
    PARAMETER, 
    LOCAL, 
    CLOSURE
} scope_t;

struct Type;
struct Expression;

struct Binding {
    scope_t scope;
    Ptr<Type> type;
    Sym name;
    Sym qualified_name;
    
    Ptr<Expression> value;
    Ptr<Binding> parent;
    
    Binding(const Binding &b)
        : scope(b.scope)
        , parent(b.parent)
        , type(b.type)
        , name(b.name)
        , qualified_name(b.qualified)
        , value(b.value) {}
    Binding(const Binding &parent, scope_t scope) 
        : scope(scope)
        , parent(&parent)
        , type(parent.type)
        , name(parent.name)
        , qualified_name(parent.qualified)
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
    // virtual Binding *find(Sym name);
};

struct Proc : Env {
    std::vector<Binding *> closure;
    std::vector<Binding *> parameters; 
    virtual Binding *find(Sym name);
};

// assignment

