// gcc vm.c -std=c99

#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>

#include "heap.h"

//typedef int word_t;

#define READI32 ((getc(in) << 24) | (getc(in) << 16) | (getc(in) << 8) | getc(in))
#define STACKSIZE 65536
#define REGSSIZE 5

word_t stack[STACKSIZE];

#include "opcodes.h"

#define ILLEGALINST {fprintf(stderr, "illegal instruction %d\n", opcode); assert(0);}

word_t WI(nint value) {
    word_t a;
    a.i = value;
    return a;
}

word_t WP(word_t * ptr) {
    word_t a;
    a.p = (void *) ptr;
    return a;
}

/*inline word_t wp(void * value) {
    word_t a;
    a.p = value;
    return a;
}*/


/*
op0
op1
op2
---
arg3
arg2
arg1
arg0 -- fp
loc0
loc1
loc2
regs -- regs
op0
op1 -- sp
*/


//#define WP(val) (wp((void *) val))

// for each proc: args size, locals size. ops size??

#define COMPUTED_GOTO 1



#if COMPUTED_GOTO
    #define CASE_OPC(opcode) case_##opcode
    
    #define JUMP_TABLE_DEF JUMP_TABLE
    
    #define JUMP_TABLE_START(opcode) goto *jmp_tbl[opcode];
    #define JUMP_TABLE_END jump_table_end:
    #define BREAK goto jump_table_end;
    
#else   
    #define CASE_OPC(opcode) case O_##opcode
    #define JUMP_TABLE_DEF ;
    #define JUMP_TABLE_START(opcode) switch(opcode) {
    #define JUMP_TABLE_END }
    #define BREAK break;
#endif 

#define TOP stack[sp]
#define POP stack[sp++]
#define PUSH(x) sp--; TOP = x
#define LOC(x) stack[fp - x - 1] 
// should be stack[fp - x - sizeof read] 
#define ARG(x) stack[fp + x]

#define TOPI stack[sp].i
#define POPI stack[sp++].i
#define PUSHI(x) sp--; TOP = WI(x)
#define LOCI(x) stack[fp - x - 1].i
#define ARGI(x) stack[fp + x].i

#define CASE_OP(op, d) CASE_OPC(op): {d;} BREAK
#define CASE_OP1I(op, d) CASE_OPC(op): TOPI = d TOPI; BREAK
//#define CASE_OP2I(op, d) CASE_OPC(op): tmp = POPI; TOPI = TOPI d tmp; BREAK
#define CASE_OP2I(op, d) CASE_OPC(op): tmp = POPI; TOPI = tmp d TOPI; BREAK

#define FETCH { \
    opcode = code[pc++]; \
    arg = 0; \
    for(int i = 0; i < o_size[opcode]; i++) arg = (arg << 8) | code[pc++]; \
}

// proc layout
// 4 bytes nargs
// 4 bytes nlocs
// NOTYET 4 bytes nops

#define R_PROC 4
#define R_PC 3
#define R_NEXT 2
#define R_FP 1
#define R_SP 0


#define GETI32(src, pos) \
    (((src)[pos] << 24) | ((src)[pos + 1] << 16) | ((src)[pos + 2] << 8) | (src)[pos + 3])

#define CALLPROC(p, retaddr) { \
    code = procmem + p; \
    nargs = GETI32(code, P_NARGS); \
    nlocs = GETI32(code, P_NLOCS); \
    psize = GETI32(code, P_PSIZE); \
    code += PROCSTART; \
    \
    stack[regs + R_PROC].i = proc; \
    stack[regs + R_PC].i = retaddr; \
    stack[regs + R_NEXT].i = next; \
    stack[regs + R_FP].i = fp; \
    stack[regs + R_SP].i = sp + nargs; \
    \
    next = regs; \
    proc = p; \
    pc = 0; \
    fp = sp; \
    regs = sp - nlocs - REGSSIZE; \
    sp = regs; \
    \
    if(debugmode) fprintf(stderr, "C proc %d pc %d next %d fp %d sp %d regs %d\n", proc, pc, next, fp, sp, regs);\
}

#define RESTORE { \
    regs = next; \
    \
    proc = stack[regs + R_PROC].i; \
    pc = stack[regs + R_PC].i; \
    fp = stack[regs + R_FP].i; \
    next = stack[regs + R_NEXT].i; \
    sp = stack[regs + R_SP].i; \
    \
    code = procmem + proc + PROCSTART; \
    \
    if(debugmode) fprintf(stderr, "R proc %d pc %d next %d fp %d sp %d regs %d\n", proc, pc, next, fp, sp, regs);\
}

#define CLOSPROC(cl) cl[CHUNKOFFS].i
#define CLOSSIZE(cl) (CHUNKSIZE(cl) - 1)
#define CLOSDATA(cl) (cl + CHUNKOFFS + 1)
#define CLOSARITY(cl) GETI32(procmem + CLOSPROC(cl), P_NARGS)

word_t * makeclos(nint proc, nint size, word_t * src) {
    word_t * cl = makechunk(1 + size);
    CLOSPROC(cl) = proc;
    memcpy(CLOSDATA(cl), src, size * sizeof(word_t));
    
    //fprintf(stderr, "clos proc %d size %d arity\n", CLOSPROC(cl), CLOSSIZE(cl));
    dumpchunk(cl);
    return cl;
}

word_t * extendclos(word_t * ocl, nint size, word_t * src) {
    int oldsize = CLOSSIZE(ocl);
    word_t * cl = makechunk(1 + oldsize + size);
    CLOSPROC(cl) = CLOSPROC(ocl);
    memcpy(CLOSDATA(cl) + oldsize, src, size * sizeof(word_t));
    memcpy(CLOSDATA(cl), CLOSDATA(ocl), oldsize * sizeof(word_t));
    //CLOSDATA(cl)[CHUNKSIZE(cl)]
    fprintf(stderr, "src[0]=%d size %d data %p %p\n", src[1].i, size, CLOSDATA(cl), CLOSDATA(cl) + oldsize);
    dumpchunk(cl);
    return cl;
}

// arg & 0xFFFF size
// arg >> 16 proc

#define CLOSURE { \
    word_t * cl = makeclos(pool[arg >> 16].i, arg & 0xFFFF, stack + sp); \
    sp += arg & 0xFFFF; /*pop*/ \
    PUSH(WP(cl)); \
}

/*if(top.clos + arg < top.arity) ; how to handle nullary closures? (thunks)
    clo = pop
    clo.add(pop arg elements)
    pc += sizeofappj*(arg - 1)
else if(top.clos + arg > top.arity) ; double call!!
    clo = pop
    push(clo.args) ; preserve order! memcpy
    call clo.fun BUT retaddress is pc + sizeofappj*(the difference - 1)
else ; total application
    clo = pop
    push(clo.args)
    call clo.fun BUT retaddress is just after the appj table...

ldi 2
ldi 1
ldi 0
ldl 0 closure arity2 size 1
appj 3
appj 2
appj 1
*/

#define SIZEOFOPC(opcode) (o_size[opcode] + 1)

#define APPLYJUMP { \
    word_t * cl = ((word_t *)POP.p), * ncl; \
    int callarity = CLOSSIZE(cl) + arg, realarity = CLOSARITY(cl); \
    if(callarity < realarity) { \
        /*fprintf(stderr, "top(%d) = %d\n", sp, TOPI);*/ \
        ncl = extendclos(cl, arg, stack + sp); \
        sp += arg; \
        PUSH(WP((void *) ncl)); \
        /* skip to the end of the appj table */ \
        pc += SIZEOFOPC(O_appj) * (arg - 1); /* should be opcode, not O_appj...*/ \
    } else { \
        /*fprintf(stderr, "top(%d) = %d\n", sp, TOPI);*/ \
        sp -= CLOSSIZE(cl); \
        memcpy(stack + sp, CLOSDATA(cl), CLOSSIZE(cl) * sizeof(word_t)); \
        /*fprintf(stderr, "calling proc %d size %d arity %d called %d stack %d\n", CLOSPROC(cl), CLOSSIZE(cl), CLOSARITY(cl), arg, sp);*/ \
        \
        CALLPROC(CLOSPROC(cl), pc + SIZEOFOPC(O_appj)*(arg - 1 - (callarity - realarity))); \
    } \
}

/*if(callarity > realarity) { \
        } else /*expected arity, skip over the table/ { \
            CALLPROC(CLOSPROC(cl), pc + SIZEOFOPC(O_appj)*(arg - 1)); \
        } \
*/
#define I_TABLE(x) \
    switch(x) { \
    case I_PUTC: \
        putchar(POPI); \
        break; \
    case I_GETC: \
        PUSHI(getchar()); \
        break; \
    case I_ERR: \
        fprintf(stderr, "Fatal error!\n"); \
        return -1; \
        break; \
    }



int main(int argc, char *argv[]) {
    assert(sizeof(nint) == sizeof(word_t) && sizeof(void *) == sizeof(word_t));
    
    int opt = 0;
    
    int debugmode = 0;

    while((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd': //debug mode enabled
                debugmode = 1;
                break;
            default:
                puts("wtf");
        }
    }
    
    assert (optind < argc);
    
    FILE *in = fopen(argv[optind], "r");
    
    unsigned int poolsize = READI32;
    unsigned int procsize = READI32;
    unsigned int rawsize = READI32;
    unsigned int entry = READI32;
    
    int poolt[poolsize];
    word_t pool[poolsize];
    char procmem[procsize];
    unsigned char rawmem[rawsize];
    const unsigned char * code;
    
    if(debugmode) fprintf(stderr, "RUNNING %d %d %d %d::::::::::\n", poolsize, procsize, rawsize, entry);
    
    for(int i = 0; i < poolsize; i++) {
        poolt[i] = getc(in);
        pool[i].i = READI32;
    }
    
    for(int i = 0; i < procsize; i++)
        procmem[i] = getc(in);
    
    for(int i = 0; i < rawsize; i++)
        rawmem[i] = getc(in);
        
    nint proc, pc, next, fp, sp, regs;
    nint rs;
    
    nint opcode, tmp, arg;
    
    word_t wtmp;
    
    nint nargs, nlocs, psize;
    
    word_t * tmpptr;
    
    proc = pool[entry].i;
    code = procmem + proc;
    nlocs = GETI32(code, P_NLOCS);
    nargs = GETI32(code, P_NARGS);
    code += PROCSTART;
    
    next = 0;
    pc = 0;
    fp = STACKSIZE;
    regs = STACKSIZE - REGSSIZE - nlocs - nargs;
    sp = regs;
    
    
    JUMP_TABLE_DEF
    
    while(1) {
        if(debugmode) printf("%5d.%5d %5d(%5d): ", proc, pc, sp, sp != regs ? TOPI : -99);
        
        FETCH
        
        if(debugmode) {
            printf("%s", o_names[opcode]);
            if(o_size[opcode]) printf(" %d", arg);
            puts("");
        }
        
        // execute
        
        JUMP_TABLE_START(opcode)

        CASE_OP(nop, ;)
        
        CASE_OP(ldl, if(debugmode) printf("loaded %d @%p = %d\n", arg, &LOC(arg), LOC(arg).i); PUSH(LOC(arg));)
        CASE_OP(stl, LOC(arg) = POP; if(debugmode) printf("stored %d @%p = %d\n", arg, &LOC(arg), LOC(arg).i);)
        CASE_OP(lda, if(debugmode) printf("loaded %d @%p = %d\n", arg, &ARG(arg), ARG(arg).i); PUSH(ARG(arg));)
        CASE_OP(sta, ARG(arg) = POP; if(debugmode) printf("stored %d @%p = %d\n", arg, &ARG(arg), ARG(arg).i);)
        CASE_OP(ldf, ILLEGALINST)
        CASE_OP(stf, ILLEGALINST)
        CASE_OP(ldc, PUSH(pool[arg]);)
        CASE_OP(ldi, PUSHI(arg);)
        
        CASE_OP(popq, ILLEGALINST)
        CASE_OP(popl, ILLEGALINST)
        CASE_OP(pop, POP;)
        CASE_OP(dup, wtmp = TOP; PUSH(wtmp);)
        CASE_OP(red, wtmp = POP; sp += arg; PUSH(wtmp);)
        
        CASE_OP2I(addi, +)
        CASE_OP2I(subi, -)
        CASE_OP2I(muli, *)
        CASE_OP2I(divi, /)
        CASE_OP2I(modi, %)
        
        CASE_OP2I(cgti, >)
        CASE_OP2I(clti, <)
        CASE_OP2I(cgei, >=)
        CASE_OP2I(clei, <=)
        CASE_OP2I(ceqi, ==)
        CASE_OP2I(cnei, !=)
        
        CASE_OP2I(andi, &)
        CASE_OP2I(ori, |)
        CASE_OP1I(noti, ~)
        CASE_OP1I(lnoti, !)
        
        CASE_OP(brt, if(POPI) pc = arg;)
        CASE_OP(brf, if(!POPI) pc = arg;)
        CASE_OP(bra, pc = arg;)
        
        CASE_OP(call, CALLPROC(pool[arg].i, pc);)
        CASE_OP(clo, CLOSURE;)
        CASE_OP(appj, APPLYJUMP;)
        CASE_OP(appv, 
            {word_t * cl = ((word_t *)POP.p); 
                CALLPROC(CLOSPROC(cl), pc);})
        
        CASE_OP(retq, ILLEGALINST )
        CASE_OP(retl, ILLEGALINST )
        CASE_OP(ret, if(next == 0) goto halt; tmp = POPI; RESTORE; PUSHI(tmp); )
        CASE_OP(retv, if(next == 0) goto halt; RESTORE; )
        
        CASE_OP(int, I_TABLE(arg);)
        
        CASE_OP(chunk, 
            //fprintf(stderr, "chunk %d\n", opcode);
            tmpptr = makechunk(arg); 
            memcpy(tmpptr + CHUNKOFFS, stack + sp, arg * sizeof(word_t)); 
            sp += arg;
            PUSH(WP(tmpptr));)
            
        CASE_OP(unchunk, 
            //fprintf(stderr, "unchunk %d\n", opcode);
            tmpptr = POP.p;
            sp -= arg;
            memcpy(stack + sp, tmpptr + CHUNKOFFS, arg * sizeof(word_t));)

        JUMP_TABLE_END
        4+3;
    }
    
    halt:
        if(debugmode) puts("end");
}
