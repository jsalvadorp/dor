#pragma once

#include <unordered_map>
#include <vector>
#include <initializer_list>
#include "ast.hpp"
#include "types.hpp"

using ast::Ptr;
using ast::Atom;
using ast::List;

enum scope_t {
    EXTERN,
    GLOBAL, 
    PARAMETER, 
    LOCAL, 
    CLOSURE
};


// NOTE consider distinguishing between const and final
enum mutability_t {
    CONST, // constant. for extern & global, this means known at compile/link/load time.
    VAR    // mutable
};

/*enum storage_t { // completely determined by scope!
    AUTO, // stack variables
    STATIC // 
};*/

struct Type;
struct Expression;

// DO NOT Ptr<Binding> !!!
struct Binding {
    scope_t scope;
    mutability_t mut;  
    
    Ptr<Type> type;
    Sym name;
    Sym qualified_name;
    
    Ptr<Expression> value;
    Ptr<Binding> parent;
    
    bool defined;
    
    int id;
    
    Binding() {}
    Binding(const Binding &b)
        : scope(b.scope)
        , parent(b.parent)
        , type(b.type)
        , name(b.name)
        , qualified_name(b.qualified_name)
        , defined(b.defined)
        , value(b.value) {}
    Binding(Binding &parent, scope_t scope) 
        : scope(scope)
        , parent(&parent)
        , type(parent.type)
        , name(parent.name)
        , defined(parent.defined)
        , qualified_name(parent.qualified_name)
        , value(parent.value) {}
        
    
};

struct Env : TypeEnv {
    Ptr<Env> parent;
    std::unordered_map<Sym, Binding> dictionary;
    
    virtual Binding *find(Sym name);
    
    Env(Ptr<Env> parent = nullptr) : parent(parent), TypeEnv(parent) {}
    
    virtual bool isGlobal() {return false;}
};

struct Globals : Env {
    std::vector<Binding *> externs; 
    
    Globals() : Env() {}
    // handle externs
    virtual Binding *find(Sym name);
    
    virtual bool isGlobal() {return true;}
};

struct Expression {
    int line, column;
    Ptr<Type> type;
    
    Expression(int line = -1, int column = -1) 
        : line(line), column(column), type(newPtr<TypeVar>(K1)) {}
    
     virtual void dump(int level) = 0;
};

struct Function : Expression, Env {
    std::vector<Binding *> closure;
    std::vector<Binding *> parameters; 
    virtual Binding *find(Sym name);
    
    Ptr<Expression> body;
    
    Function(Ptr<Env> parent) : Env(parent) {} 
    
    virtual void dump(int level) {
        ast::print_indent(level);
        std::cout << "FUNCTION : ";
        type->dump();
        std::cout << std::endl;
        
        ast::print_indent(level + 1);
        std::cout << "CLOSURE ";
        for(Binding *b : closure) {
            std::cout << b->name.str() << " ";
        }
        std::cout << std::endl;
        
        ast::print_indent(level + 1);
        std::cout << "PARAMS ";
        for(Binding *b : parameters) {
            std::cout << b->name.str() << " ";
        }
        std::cout << std::endl;
        
        body->dump(level + 1);
    }
};

struct Literal : Expression {
    Ptr<Atom> value;
    
    Literal(Ptr<Atom> value, Ptr<Type> type) : value(value) {
        line = value->line;
        column = value->column;
        this->type = type;
    }
    
    virtual void dump(int level) {
        value->dump(level);
        std::cout << std::endl;
    }
};

struct Reference : Expression { // lvalue/rvalue distinciton!!!
    Binding *binding;
    
    Reference(Binding *b) : binding(b) {
        type = b->type;
    }
    
    
    virtual void dump(int level) {
        ast::print_indent(level);
        
        switch(binding->scope) {
        case PARAMETER: std::cout << "PARAM("; break;
        case CLOSURE: std::cout << "CLOSURE("; break;
        case LOCAL: std::cout << "LOCAL("; break;    
        case GLOBAL: std::cout << "GLOBAL("; break;  
        case EXTERN: std::cout << "EXTERN("; break;    
        }
        
        std::cout << binding->name.str() << ") : ";
        binding->type->dump();
        std::cout << std::endl;
    }
};

struct Application : Expression {
    Ptr<Expression> left, right;
    
    Application(Ptr<Expression> left, Ptr<Expression> right)
        : left(left), right(right) {
        assert(this->left!=nullptr);
        assert(this->right!=nullptr);
        type = applyTypes(left->type, right->type);
    }
    
    virtual void dump(int level) {
        ast::print_indent(level);
        std::cout << "APPLY : ";
        type->dump();
        std::cout << std::endl;
        
        left->dump(level + 1);
        right->dump(level + 1);
    }
};

struct Assignment : Expression {
    Ptr<Reference> lhs; // replace by generic lvalue reference??
    Ptr<Expression> rhs;
    
    bool is_const_expr; // 
    
    Assignment(Ptr<Reference> lhs, Ptr<Expression> rhs)
        : lhs(lhs), rhs(rhs), Expression() {
        assert(this->lhs!=nullptr);
        assert(this->rhs!=nullptr);
        
        type = newPtr<TypeVar>(K1);
        assert(unifyTypes(type, this->lhs->type));
        assert(unifyTypes(type, this->rhs->type));
    }
    
    virtual void dump(int level) {
        ast::print_indent(level);
        std::cout << "ASSIGN" << std::endl;
        
        lhs->dump(level + 1);
        rhs->dump(level + 1);
    }
};

struct Conditional : Expression {
    Ptr<Expression> condition, on_false, on_true;
    
    Conditional(Ptr<Expression> condition, Ptr<Expression> on_true, Ptr<Expression> on_false)
        : condition(condition), on_false(on_false), on_true(on_true) {
        type = newPtr<TypeVar>(K1);
        assert(unifyTypes(condition->type, Bool));
        assert(unifyTypes(type, on_true->type));
        assert(unifyTypes(type, on_false->type));
    }
    Conditional() {}
    
    virtual void dump(int level) {
        ast::print_indent(level);
        std::cout << "CONDITIONAL" << std::endl;
        
        condition->dump(level + 1);
        on_true->dump(level + 1);
        on_false->dump(level + 1);
    }
};

struct Sequence : Expression {
    Ptr<Env> locals;
    std::vector<Ptr<Expression> > steps;
    
    Sequence(Ptr<Env> parent) : locals(newPtr<Env>(parent)) {}
    Sequence() {}
    
    virtual void dump(int level) {
        ast::print_indent(level);
        std::cout << "SEQUENCE" << std::endl;
        
        for(Ptr<Expression> exp : steps) {
            exp->dump(level + 1);
        }
    }
};

struct MatchClause {
    Ptr<Env> env;
    Ptr<Expression> pattern, condition, body;
};

struct Match : Expression {
    std::vector<MatchClause> clauses;
    
    void dump(int level) {}
};

Ptr<Expression> topLevel(Ptr<Globals> globals, Ptr<Atom> exp);
Ptr<Sequence> program(Ptr<Globals> env, Ptr<List> body);

void initGlobals(Ptr<Globals> g);
