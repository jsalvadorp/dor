
#include <cassert>
#include <iomanip>
#include "analysis.hpp"
#include "codegen.hpp"
#include "ast.hpp"


#include "instructions.hpp"


instructions::instr instructions::iset[256];

#define OPCODE(name) instructions::name::code

// TODO: constant folding

using ast::i64;

enum GlobalType {
    // the constants that will be in-place
    LongGlobal,
    DoubleGlobal,
    
    // the constants that will point to code/data sections
    StringGlobal,
    ProcGlobal
};


struct Global {
    virtual GlobalType type() = 0;
    virtual void write() = 0;
    
    virtual void disassemble() = 0;
};

struct DoubleG : Global {
    virtual GlobalType type() {return DoubleGlobal;}
    virtual void write() {}
    
    double value;
    
    DoubleG(double val) : value(val) {}
    virtual void disassemble() {
        std::cout << " FLOAT {" << value << "}" << std::endl;
    }
};

struct LongG : Global {
    virtual GlobalType type() {return DoubleGlobal;}
    virtual void write() {}
    
    i64 value;
    int size;
    
    LongG(i64 val, int sz = 8) : value(val), size(sz) {}
    
    virtual void disassemble() {
        std::cout << " LONG [" << size << "] {" << value << "}" << std::endl;
    }
};

struct StringG : Global {
    virtual GlobalType type() {return DoubleGlobal;}
    virtual void write() {}
    
    std::string value;
    
    StringG(const std::string &val) : value(val) {}
    
    virtual void disassemble() {
        std::cout << " STRING {" << value << "}" << std::endl;
    }
};

struct Proc : Global {
    unsigned int arity;
    unsigned int locals;
    unsigned int max_ops; // unused
    unsigned int unknown;
    
    unsigned int ops;
    
    std::vector<unsigned char> code;
    
    virtual GlobalType type() {
        return ProcGlobal;
    }
    
    virtual void write() {}
    
    Proc() {}
    
    unsigned int getOps() {return ops;}
    void setOps(unsigned int o) {
        ops = o;
        max_ops = std::max(max_ops, ops);
    }
    
    void incOps() {setOps(getOps() + 1);}
    void decOps() {assert(ops); setOps(getOps() - 1);}
    
    virtual void disassemble() {
        std::cout << " PROC " << std::endl;
        std::cout << " ARITY " << arity << std::endl;
        std::cout << " LOCALS " << locals << std::endl;
        std::cout << " OPS " << max_ops << std::endl;
        std::cout << " CODE " << std::endl;
        for(int i = 0; i < code.size(); i++) {
            int inst = code[i];
            int arg0s = instructions::iset[inst].arg0_size;
            int arg1s = instructions::iset[inst].arg1_size;
            
            i64 arg0 = 0, arg1 = 0;
            
            std::cout << "   " << std::setw(4) << i << " " << instructions::iset[inst].name;
            if(arg0s) {
                while(arg0s--) {
                    arg0 |= ((unsigned int)code[++i] << (arg0s)) & 0xFF;
                }
                std::cout << " " << arg0;
            }
            if(arg1s) {
                while(arg1s--) {
                    arg1 |= ((unsigned int)code[++i] << (arg1s)) & 0xFF;
                }
                std::cout << " " << arg1;
            }
            std::cout << std::endl;
        }
    }
};




std::vector<Ptr<Global> > pool; // constant pool
Ptr<Proc> load;



struct Compile : ExpressionVisitor {
    Ptr<Proc> surrounding;
    
    bool is_tail; // is in the tail position of a function
    bool ignore_result; // assignment should not leave a value on stack
    bool no_undefined; // no global undefined references (at load time)
    
    Compile(Ptr<Proc> s, bool t = false, bool p = false, bool n = false) 
        : surrounding(s)
        , is_tail(t)
        , ignore_result(p)
        , no_undefined(n) {}
    
    virtual void visit(Function &f) {
        Ptr<Proc> new_proc;
        int id;
        
        if(f.id >= 0) {
            new_proc = castPtr<Proc, Global>(pool[f.id]);
            // new_proc->id 
            id = f.id;
        } else {
            new_proc = newPtr<Proc>();
            // new_proc->id 
            id = pool.size();
            pool.push_back(new_proc);
        }
        
        // TODO: handle hidden vtable params
        new_proc->arity = f.parameters.size() + f.closure.size(); 
        new_proc->locals = f.max_locals;
        new_proc->max_ops = 0; // unused (for now)
        new_proc->unknown = 0; 
        
        new_proc->ops = 0;
        
        Compile comp(new_proc);
        comp.is_tail = true;
        f.body->accept(comp);
        assert(new_proc->ops == 0);
        
        // load in reverse onto the stack
        // TODO: handle vtable hidden params
        for(int i = f.closure.size() - 1; i >= 0; i--) {
            Binding *parent = f.closure[i]->parent;
            assert(parent);
            
            compileBinding(parent);
        }
        
        if(surrounding != nullptr) {
            if(!ignore_result) {
                assert(f.closure.size() <= 255); // for now, the limit on arguments
                if(f.closure.size()) {
                    emit(OPCODE(clos21), id, f.closure.size());
                } else
                    emit(OPCODE(ldc2), id);
                    
                if(is_tail)
                    emit(OPCODE(ret));  
            }
        } else assert(f.closure.size() == 0);
        
        
    }
    
    void compileBinding(Binding *binding, bool store = false, bool load_time = false) {
        if(no_undefined) 
            assert(binding->scope != GLOBAL || binding->defined);
        
        if(store) {
            // during load_time, it is permitted to assign to consts
            assert(binding->mut != CONST || no_undefined);
            
            switch(binding->scope) {
            case GLOBAL:
            case EXTERN:
                if(binding->mut == CONST) { // only allowed in load
                    emit(OPCODE(stc2), binding->id);
                } else
                    emit(OPCODE(stg2), binding->id);
                break;
            case LOCAL:
                emit(OPCODE(stl), binding->id);
                break;
            case PARAMETER:
            case CLOSURE:
                emit(OPCODE(sta), binding->id);
                break;
            }
        } else {
            switch(binding->scope) {
            case GLOBAL:
            case EXTERN:
                if(binding->mut == CONST) {
                    /*Ptr<Function> f;
                    if(binding->value == nullptr
                        && (f = castPtr<Function, Expression>(binding->value))) {
                        assert(f->id = binding->id && f->id >= 0);
                        emit(OPCODE(clos2), binding->id);
                    } else */
                    emit(OPCODE(ldc2), binding->id);
                } else
                    emit(OPCODE(ldg2), binding->id);
                break;
            case LOCAL:
                emit(OPCODE(ldl), binding->id);
                break;
            case PARAMETER:
            case CLOSURE:
                emit(OPCODE(lda), binding->id);
                break;
            }
        }
    }
    
    size_t emit(unsigned int code, unsigned long arg0 = 0, unsigned long arg1 = 0) {
        assert(code != OPCODE(retv));
        
        surrounding->code.push_back(code);
        size_t arg0s = instructions::iset[code].arg0_size,
               arg1s = instructions::iset[code].arg1_size;
        
        /*std::cout << instructions::iset[code].name;
        if(arg0s) std::cout << " " << arg0;
        if(arg1s) std::cout << " " << arg1;
        std::cout << std::endl;*/
        
        size_t pos_arg0 = surrounding->code.size();
        
        while(arg0s--) {
            surrounding->code.push_back((arg0 >> (8 * arg0s)) & 0xFF);
        }
        
        while(arg1s--) {
            surrounding->code.push_back((arg1 >> (8 * arg1s)) & 0xFF);
        }
        
        
        
        // check the stack!
        int arity;
        
        switch(code) {
        case OPCODE(red):
            assert(surrounding->getOps() >= arg0);
            surrounding->setOps(surrounding->getOps() - arg0 + 1);
            break;
        case OPCODE(call2):
            assert(arg0 < pool.size() 
                && pool[arg0] != nullptr
                && pool[arg0]->type() == ProcGlobal);
            arity = castPtr<Proc, Global>(pool[arg0])->arity;
            assert(surrounding->getOps() >= arity);
            surrounding->setOps(surrounding->getOps() - arity + 1); 
            break;
        case OPCODE(clos21):
            assert(arg0 < pool.size() 
                && pool[arg0] != nullptr
                && pool[arg0]->type() == ProcGlobal);
            arity = castPtr<Proc, Global>(pool[arg0])->arity;
            assert(arity >= arg1);
            assert(surrounding->getOps() >= arg1);
            surrounding->setOps(surrounding->getOps() - arg1 + 1); 
            break;
        case OPCODE(app):
            assert(arg0);
            assert(surrounding->getOps() >= arg0);
            surrounding->setOps(surrounding->getOps() - 1); 
            break;
        case OPCODE(chunk):
            assert(surrounding->getOps() >= arg0);
            surrounding->setOps(surrounding->getOps() - arg0 + 1);
            break;
        case OPCODE(unchunk):
            assert(surrounding->getOps() >= 1);
            surrounding->setOps(surrounding->getOps() + arg0);
            break;
        // all other cases behave the same: their iset specifies before/after
        default:
            assert(surrounding->getOps() >= instructions::iset[code].before);
            surrounding->setOps(
                surrounding->getOps()
                - instructions::iset[code].before 
                + instructions::iset[code].after);
        }
        
        return pos_arg0;
    }
    
    void putValue(unsigned int pos, unsigned int len, unsigned long val) {
        while(len--) {
            surrounding->code[pos++] = ((val >> (8 * len)) & 0xFF);
        }
    }
    
    virtual void visit(Literal &e) {
        // for now, integers are stored in the code, floats are moved
        // to the pool.
        switch(e.value->type()) {
        case ast::BoolType:
            emit(OPCODE(ldi), e.value->get_bool() ? 1 : 0);
            break;
        case ast::VoidType:
            emit(OPCODE(ldi), 0);
            break;
        case ast::I64Type:
            emit(OPCODE(ldi8), e.value->get_i64());
            break;
        case ast::CharType:
            emit(OPCODE(ldi), e.value->get_char());
            break;
        case ast::DoubleType:
            pool.push_back(newPtr<DoubleG>(e.value->get_double()));
            emit(OPCODE(ldc2), pool.size() - 1);
            break;
        case ast::StringType:
            pool.push_back(newPtr<StringG>(e.value->get_string()));
            emit(OPCODE(ldc2), pool.size() - 1);
            break;
        default:
            assert(!"Invalid literal");
        }
        
        if(is_tail)
            emit(OPCODE(ret));
    }
    
    virtual void visit(Reference &e) {
        compileBinding(e.binding);
        
        if(is_tail)
            emit(OPCODE(ret));
    }
    
    virtual void visit(Conditional &e) {
        Compile comp(surrounding);
        comp.no_undefined = no_undefined;
        
        size_t jump_false, jump_end;
        
        size_t prev_ops = surrounding->getOps();
        
        comp.is_tail = false;
        e.condition->accept(comp);
        
        jump_false = emit(OPCODE(brf2), -1);
        assert(prev_ops == surrounding->getOps());
        
        comp.is_tail = is_tail;
        e.on_true->accept(comp);
        
        prev_ops = surrounding->getOps();
        
        if(!comp.is_tail) {
            jump_end = emit(OPCODE(bra2), -1);
        }
        
        putValue(jump_false, 2, surrounding->code.size());
        
        comp.is_tail = is_tail;
        e.on_false->accept(comp);
        
        assert(prev_ops == surrounding->getOps());
        
        if(!comp.is_tail) {
            putValue(jump_end, 2, surrounding->code.size());
        }
    }
    
    virtual void visit(Sequence &e) {
        Compile comp(surrounding);
        comp.no_undefined = no_undefined;
        
        size_t prev_ops = surrounding->getOps();
        
        for(size_t i = 0; i < e.steps.size(); i++) {
            Ptr<Expression> step = e.steps[i];
            
            comp.is_tail = is_tail && i == e.steps.size() - 1;
            comp.ignore_result = i < e.steps.size() - 1 && !no_undefined;
            
            assert(prev_ops == surrounding->getOps());
            step->accept(comp);
            assert(surrounding->getOps() >= prev_ops);
            
            if(!comp.is_tail) {
                while(surrounding->getOps() > prev_ops)
                    emit(OPCODE(pop));
            } else {
                // already placed a ret
                assert(surrounding->getOps() == prev_ops);
            }
        }
    }
    
    virtual void visit(Application &e) {
        Expression *leftmost = &e;
        std::vector<Ptr<Expression> > args;
        
        while(auto a = dynamic_cast<Application *>(leftmost)) {
            leftmost = a->left.get();
            args.push_back(a->right);
        }
        
        // args holds the argument in reverse order (the order they must
        // be pushed onto the stack, rightmost to leftmost)
        
        if(auto ref = dynamic_cast<Reference *>(leftmost)) {
            Sym name = ref->binding->name;
            
            // short-circuit evaluation
            if(name == Sym("&&")) 
                assert(!"Short-circuit not yet implemented");
            else if(name == Sym("||"))
                assert(!"Short-circuit not yet implemented");
            
        }
        
        Compile comp(surrounding);
        comp.no_undefined = no_undefined;
        comp.is_tail = false;
        
        for(Ptr<Expression> arg : args) {
            arg->accept(comp);
        }
        
        if(auto ref = dynamic_cast<Reference *>(leftmost)) {
            Sym name = ref->binding->name;
            
            // TODO: handle Number, Equatable interface and such
                
            // builtin functions
            if(name == "+" && args.size() == 2) {
                emit(OPCODE(iadd));
                
            } else if(name == "-" && args.size() == 2) {
                emit(OPCODE(isub));
            } else if(name == "*" && args.size() == 2) {
                emit(OPCODE(imul));
            } else if(name == "/" && args.size() == 2) {
                emit(OPCODE(idiv));
            } else if(name == "%" && args.size() == 2) {
                emit(OPCODE(imod));
            } else if(name == "<" && args.size() == 2) {
                emit(OPCODE(ilt));
            } else if(name == ">" && args.size() == 2) {
                emit(OPCODE(igt));
            } else if(name == "<=" && args.size() == 2) {
                emit(OPCODE(ile));
            } else if(name == ">=" && args.size() == 2) {
                emit(OPCODE(ige));
            } else if(name == "!=" && args.size() == 2) {
                emit(OPCODE(ine));
            } else if(name == "==" && args.size() == 2) {
                emit(OPCODE(ieq));
            } else if(name == "<." && args.size() == 2) {
                emit(OPCODE(flt));
            } else if(name == ">." && args.size() == 2) {
                emit(OPCODE(fgt));
            } else if(name == "<=." && args.size() == 2) {
                emit(OPCODE(fle));
            } else if(name == ">=." && args.size() == 2) {
                emit(OPCODE(fge));
            } else if(name == "!=." && args.size() == 2) {
                emit(OPCODE(fne));
            } else if(name == "==." && args.size() == 2) {
                emit(OPCODE(feq));
            } else if(name == "+." && args.size() == 2) {
                emit(OPCODE(fadd));
            } else if(name == "-." && args.size() == 2) {
                emit(OPCODE(fsub));
            } else if(name == "*." && args.size() == 2) {
                emit(OPCODE(fmul));
            } else if(name == "/." && args.size() == 2) {
                emit(OPCODE(fdiv));
            } else {
                Binding *b = ref->binding;
                if((b->scope == GLOBAL || b->scope == EXTERN) && b->id < pool.size()) {
                    if(Ptr<Proc> proc = castPtr<Proc, Global>(pool[b->id])) {
                        if(args.size() >= proc->arity) {
                            emit(OPCODE(call2), b->id);
                            
                            for(int i = args.size(); i < proc->arity; i++) {
                                emit(OPCODE(app), proc->arity - i);
                            }
                        } else if(args.size() < proc->arity) {
                            emit(OPCODE(clos21), b->id, args.size());
                        }
                    } else {
                        leftmost->accept(comp);
                        for(int i = args.size(); i > 0; i--) {
                            emit(OPCODE(app), i);
                        }
                    }
                } else {
                    leftmost->accept(comp);
                    for(int i = args.size(); i > 0; i--) {
                        emit(OPCODE(app), i);
                    }
                } 
            }
        } else {
            leftmost->accept(comp);
            for(int i = args.size(); i > 0; i--) {
                emit(OPCODE(app), i);
            }
        }

        
        // TAIL CALL OPTIMIZATION????
        if(is_tail) {
            emit(OPCODE(ret));
        }
    }
    
    virtual void visit(Assignment &e) {
        Ptr<Reference> lhs = e.lhs;
        Ptr<Expression> rhs = e.rhs;
        
        Compile comp(surrounding, is_tail, ignore_result, no_undefined);
        // comp.no_undefined = no_undefined;
        rhs->accept(comp);
        
        if(!ignore_result)
            emit(OPCODE(dup));
        compileBinding(lhs->binding, true);
    }
    
    virtual void visit(Match &e) {}
};

void compileConstructor(Binding *b) {
    Ptr<Type> t = b->type;
    Ptr<Proc> new_proc = newPtr<Proc>();
    
        
    assert(b->id >= 0);
    
    pool[b->id] = new_proc;
    new_proc->arity = 0;
    new_proc->unknown = 0; 
    new_proc->ops = 0;
    new_proc->max_ops = 0;
    new_proc->locals = 0; // 
    
    Compile comp(new_proc);
    comp.is_tail = true;
    
    if(Ptr<TypeForAll> tfa = castPtr<TypeForAll, Type>(t)) {
        t = tfa->right;
    }
    
    // TODO: handle hidden vtable params
    while(isFuncType(t, nullptr, &t)) {
        new_proc->arity++; // unused (for now)
    }
    
    
    for(int i = new_proc->arity - 1; i >= 0; i--) {
        comp.emit(OPCODE(lda), i);
    }
    comp.emit(OPCODE(ldc2), b->id);
    comp.emit(OPCODE(chunk), new_proc->arity + 1);
    comp.emit(OPCODE(ret));
    
    
    assert(new_proc->ops == 0);
}

void compile(Ptr<Globals> globals, Ptr<Sequence> load_seq) {
    pool = std::vector<Ptr<Global> >(
        globals->externs.size() + globals->globals.size());
    int externs = globals->externs.size();
    
    // externs and non-constexprs represented as nullptrs in the pool
    
    // TODO: raise an error when two non-constexpr global constants
    // refer to one another!
    
    int id = 0;
    
    for(Binding *b : globals->externs) {
        //pool[id++] = nullptr;
        id++;
    }
    
    
    Compile compg(nullptr);
    
    for(Binding *b : globals->globals) {
        assert(id == b->id);
        if(b->isconstexpr && b->value != nullptr) {
            if(Ptr<Function> f = castPtr<Function, Expression>(b->value)) {
                assert(f->id == id);
                //Ptr<Proc> new_proc = newPtr<Proc>();
        
                // TODO: handle hidden vtable params
                //new_proc->arity = f->parameters.size(); 
                assert(f->id != -1);
                pool[f->id] = newPtr<Proc>();
                f->accept(compg);
                
            } else if(Ptr<Literal> f = castPtr<Literal, Expression>(b->value)) {
                switch(f->value->type()) {
                case ast::BoolType:
                    pool[id] = newPtr<LongG>(f->value->get_bool(), 8);
                    break;
                case ast::VoidType:
                    pool[id] = newPtr<LongG>(0, 8);
                    break;
                case ast::I64Type:
                     pool[id] = newPtr<LongG>(f->value->get_i64(), 8);
                    break;
                case ast::CharType:
                    pool[id] = newPtr<LongG>(f->value->get_char(), 1);
                    break;
                case ast::DoubleType:
                    pool[id] = newPtr<DoubleG>(f->value->get_double());
                    break;
                case ast::StringType:
                    pool[id] = newPtr<StringG>(f->value->get_string());
                    break;    
                default:
                    assert(!"Illegal Literal!!");
                }
            }else assert(!"Invalid constexpr!");  
            
            
        }  else if(b->constructor) {
            compileConstructor(b);
        } 
        
        id++;
        
        
    }
    
    load = newPtr<Proc>();
    int load_id = pool.size();
    pool.push_back(load);
    
    load->arity = 0; //f.parameters.size(); 
    load->locals = 0;
    load->max_ops = 0; // unused (for now)
    load->unknown = 0; 
    load->ops = 0;
    
    Compile comp(load);
    comp.is_tail = false;
    comp.ignore_result = true;
    comp.no_undefined = true;
    
    //load_seq->accept(comp);
    
    for(Ptr<Expression> exp : load_seq->steps) {
        exp->accept(comp);
    }
    
    int i = 0;
    for(Ptr<Global> glo : pool) {
        if(i < globals->externs.size())
            std::cout << globals->externs[i]->name.str();
        else if(i < globals->externs.size() + globals->globals.size()) {
            std::cout << 
                globals->globals[i - globals->externs.size()]
                    ->name.str();
        }
        std::cout << "{" << i << "}" << std::endl;
        if(pool[i] != nullptr) {
            pool[i]->disassemble();
        }
        i++;
    }
}


void initCompiler() {
    instructions::initInstructions();
}
