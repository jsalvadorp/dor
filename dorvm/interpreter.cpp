#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "interpreter.hpp"
#include "loader.hpp"


#include "../dorc/instructions.hpp"
#include "heap.hpp"

instructions::instr instructions::iset[256];

#define TOP stack[sp]
#define POP stack[sp++]
#define PUSH(x) sp--; TOP = x
#define LOC(x) stack[fp - x - 1] 
#define ARG(x) stack[fp + x]
#define CASE_OPCODE(name) case instructions::name::code
#define CASE_OP1i64(op, d) CASE_OPCODE(op): TOP.i64 = d TOP.i64; break;
#define CASE_OP2i64(op, d) CASE_OPCODE(op): tmp = POP; TOP.i64 = tmp.i64 d TOP.i64; break;
#define CASE_OP2ui64(op, d) CASE_OPCODE(op): tmp = POP; TOP.ui64 = tmp.ui64 d TOP.ui64; break;
#define CASE_OP1f64(op, d) CASE_OPCODE(op): TOP.f64 = d TOP.f64; break;
#define CASE_OP2f64(op, d) CASE_OPCODE(op): tmp = POP; TOP.f64 = tmp.f64 d TOP.f64; break;
#define CASE_OP2f64(op, d) CASE_OPCODE(op): tmp = POP; TOP.f64 = tmp.f64 d TOP.f64; break;

#define REGS_SIZE 5

#define CALL(p, retaddr) \
    if(CLOS_PROC(p)->native_impl) { \
        obj::word_t *args = stack + sp; \
        sp += CLOS_PROC(p)->arity; \
        PUSH(CLOS_PROC(p)->native_impl(args)); \
        pc = retaddr; \
    } else { \
        stack[regs + 0].ptr  = (void *) proc; \
        stack[regs + 1].ui64 = retaddr; \
        stack[regs + 2].ui64 = next; \
        stack[regs + 3].ui64 = fp; \
        stack[regs + 4].ui64 = sp + CLOS_PROC(p)->arity; \
        next = regs; \
        proc = CLOS_PROC(p); \
        pc = 0; \
        fp = sp; \
        regs = sp - CLOS_PROC(p)->locals - REGS_SIZE; \
        sp = regs; \
    }

#define RESTORE \
    regs = next; \
    proc = (Proc *) stack[regs + 0].ptr; \
    pc = stack[regs + 1].ui64; \
    next = stack[regs + 2].ui64; \
    fp = stack[regs + 3].ui64; \
    sp = stack[regs + 4].ui64; \
    pool = proc->pool;

#define APPLY \
    obj::word_t *clos = (obj::word_t *) POP.ptr; \
    int argc = CLOS_SIZE(clos) + arg0; \
    int arity = CLOS_PROC(clos)->arity; \
    if(argc < arity) { \
        obj::word_t *new_clos = extendClosure(clos, stack + sp, arg0); \
        sp += arg0; \
        PUSH(wordPtr(new_clos)); \
        pc += instructions::app::size * (arg0 - 1); \
    } else { \
        sp -= CLOS_SIZE(clos); \
        std::copy(CLOS_DATA(clos), CLOS_DATA(clos) + CLOS_SIZE(clos), stack + sp); \
        CALL(clos, pc + instructions::app::size * (arg0 - 1 - (argc - arity))); \
    }

inline obj::word_t wordPtr(void *p) {
    obj::word_t w;
    w.ptr = p;
    return w;
}

#include "builtins.hpp"

NativeFunc *builtin_func_impls[] = {
    printInt,
    printFloat,
    printString,
    readInt,
    readFloat,
    readString,
    intToFloat,
    floatToInt

};

int builtin_func_impls_argc[] = {1,1,1,1,1,1};

void run(obj::word_t *stack, uint64_t stack_size, Proc *start) {
    // registers
    Proc *proc = start;

    uint64_t 
        pc = 0, 
        fp = stack_size - proc->arity,
        regs = fp - proc->locals - REGS_SIZE,
        sp = regs,
        next = 0;

    // other
    obj::word_t *pool = proc->pool;
    //unsigned char *code = proc->code;
    obj::word_t tmp, *ptr_tmp;

    while(1) {
        

        int inst = proc->code[pc++];
        int arg0s = instructions::iset[inst].arg0_size;
        int arg1s = instructions::iset[inst].arg1_size;
        
        uint64_t arg0 = 0, arg1 = 0;
        
        // sign extension!!
        // std::cerr << "   #" << std::setw(4) << (pc-1) << " " << instructions::iset[inst].name;
        
        if(arg0s) {
            while(arg0s--) {
                arg0 |= ((uint64_t)proc->code[pc++] << (8 * arg0s));
            } //std::cerr << " " << arg0;
        }
        if(arg1s) {
            while(arg1s--) {
                arg1 |= ((uint64_t)proc->code[pc++] << (8 * arg1s));
            } //std::cerr << " " << arg1;
        }
        //std::cerr << " [" << TOP.i64 << "__" << TOP.f64 << "] sp = " << sp;
        //std::cerr << std::endl;
        
        switch(inst) {
        CASE_OPCODE(halt):
            return;
        CASE_OPCODE(nop):
            break;
        CASE_OPCODE(pop):
            POP;
            break;
        CASE_OPCODE(dup):
            tmp= TOP;
            PUSH(tmp);
            break;
        CASE_OPCODE(red):
            tmp = POP;
            sp += arg0;
            PUSH(tmp);
            break;
        CASE_OPCODE(ldi): // sign extension!!!
        CASE_OPCODE(ldi2):
        CASE_OPCODE(ldi4):
        CASE_OPCODE(ldi8):
            tmp.i64 = arg0;
            PUSH(tmp);
            break;
        CASE_OPCODE(lda):
            PUSH(ARG(arg0));
            break;
        CASE_OPCODE(sta):
            ARG(arg0) = POP;
            break;
        CASE_OPCODE(ldl):
            PUSH(LOC(arg0));
            break;
        CASE_OPCODE(stl):
            LOC(arg0) = POP;
            break;
        // for now, ldc = ldg and stg = stc.
        CASE_OPCODE(ldc2):
        CASE_OPCODE(ldg2):
            PUSH(pool[arg0]);
            break;
        CASE_OPCODE(stc2):
        CASE_OPCODE(stg2):
            pool[arg0] = POP;
            break;
        CASE_OPCODE(ldf):
            ptr_tmp = (obj::word_t *) POP.ptr;
            PUSH(ptr_tmp[arg0]);
            break;
        CASE_OPCODE(stf):
            ptr_tmp = (obj::word_t *) POP.ptr;
            ptr_tmp[arg0] = POP;
            break;
        CASE_OPCODE(ldfc2):
            ptr_tmp = (obj::word_t *) POP.ptr;
            PUSH(ptr_tmp[pool[arg0].ui64]);
            break;
        CASE_OPCODE(stfc2):
            ptr_tmp = (obj::word_t *) POP.ptr;
            ptr_tmp[pool[arg0].ui64] = POP;
            break;
        
        // arithmetic



        CASE_OP2i64(iadd, +);
        CASE_OP2i64(isub, -);
        CASE_OP2i64(imul, *);
        CASE_OP2i64(idiv, /);
        CASE_OP2i64(imod, %);

        CASE_OP2f64(fadd, +);
        CASE_OP2f64(fsub, -);
        CASE_OP2f64(fmul, *);
        CASE_OP2f64(fdiv, /);

        CASE_OP1i64(inot, ~);
        CASE_OP2i64(iand, &);
        CASE_OP2i64(ior, |);
        CASE_OP2i64(ixor, ^);
        CASE_OP2i64(ishl, <<);
        CASE_OP2i64(ishr, >>);
        CASE_OP2ui64(ishru, >>);
        CASE_OP1i64(ilnot, !);

        CASE_OP2i64(igt, >);
        CASE_OP2i64(ilt, <);
        CASE_OP2i64(ige, >=);
        CASE_OP2i64(ile, <=);
        CASE_OP2i64(ieq, ==);
        CASE_OP2i64(ine, !=);

        CASE_OP2f64(fgt, >);
        CASE_OP2f64(flt, <);
        CASE_OP2f64(fge, >=);
        CASE_OP2f64(fle, <=);
        CASE_OP2f64(feq, ==);
        CASE_OP2f64(fne, !=);
        
        CASE_OPCODE(brt2):
            if(POP.i64)
                pc = arg0;
            break;
        CASE_OPCODE(brf2):
            if(!POP.i64)
                pc = arg0;
            break;
        CASE_OPCODE(bra2):
            pc = arg0;
            break;

        CASE_OPCODE(call2):
            CALL((obj::word_t *)pool[arg0].ptr, pc);
            break;
        CASE_OPCODE(clos21): {
            Proc *p = CLOS_PROC((obj::word_t *)pool[arg0].ptr); 
            obj::word_t *c = makeClosure(p, stack + sp, arg1);
            sp += arg1;
            PUSH(wordPtr((void *)c));
            break;
        }
        CASE_OPCODE(app): {
            APPLY;
            break;
        }
        CASE_OPCODE(ret):
            if(next == 0) return;
            tmp = POP;
            RESTORE;
            PUSH(tmp);
            break;
        CASE_OPCODE(retv):
            if(next == 0) return;
            RESTORE;
            break;
        CASE_OPCODE(chunk): {
            obj::word_t *ch = makeChunk(stack + sp, arg0);
            sp += arg0;
            PUSH(wordPtr((void *)ch));
            break;
        }
        CASE_OPCODE(sys):
            assert(arg0 < sizeof(builtin_func_impls) / sizeof(NativeFunc *));
            tmp = builtin_func_impls[arg0](stack + sp);
            sp += builtin_func_impls_argc[arg0];
            PUSH(tmp);
            break;
        CASE_OPCODE(fail): {
            std::cerr << "Pattern match failed" << std::endl;
            assert(0);
            break;

        }

        }
    }

}




