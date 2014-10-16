
namespace instructions {
    struct instr {
        const char *name;
        unsigned int arg0_size;
        unsigned int arg1_size;
        unsigned int before;
        unsigned int after;
        
        instr() 
            : name(nullptr)
            , arg0_size(-1)
            , arg1_size(-1)
            , before(-1)
            , after(-1) {}
        instr(const char *n, unsigned int a0, 
                unsigned int a1, unsigned int bf, unsigned int af) 
            : name(n)
            , arg0_size(a0)
            , arg1_size(a1)
            , before(bf)
            , after(af)  {}
    };
    
    extern instr iset[256];
    
    template<size_t Code>
    inline instr getInstr() { return iset[Code] = instr(); }
    
    #define DEFINE_INSTRUCTION(NAME, CODE, ARG0, ARG1, BEFORE, AFTER) \
    struct NAME { \
        enum { \
            code = CODE, \
            arg0_size = ARG0, \
            arg1_size = ARG1, \
            before = BEFORE, \
            after = AFTER, \
        }; \
        static constexpr const char *name = #NAME; \
    }; \
    template<> \
    inline instr getInstr<CODE>() { \
        return iset[CODE] = instr(#NAME, ARG0, ARG1, BEFORE, AFTER); \
    }
    
    // 
    DEFINE_INSTRUCTION(halt,    0x00,  0,  0,  0,  0);
    DEFINE_INSTRUCTION(nop,     0x01,  0,  0,  0,  0);
    
    // stack transformation
    DEFINE_INSTRUCTION(pop,     0x04,  0,  0,  1,  0);
    DEFINE_INSTRUCTION(dup,     0x05,  0,  0,  1,  2);
    DEFINE_INSTRUCTION(red,     0x06,  1,  0,  -1, 1);
    
    // load-store
    DEFINE_INSTRUCTION(ldi,     0x10,  1,  0,  0,  1); // immediate value
    DEFINE_INSTRUCTION(ldi2,    0x11,  2,  0,  0,  1);
    DEFINE_INSTRUCTION(ldi4,    0x12,  4,  0,  0,  1);
    DEFINE_INSTRUCTION(ldi8,    0x13,  8,  0,  0,  1);
    DEFINE_INSTRUCTION(lda,     0x14,  1,  0,  0,  1); // argument variable
    DEFINE_INSTRUCTION(sta,     0x15,  1,  0,  1,  0);
    DEFINE_INSTRUCTION(ldl,     0x16,  1,  0,  0,  1); // local variable
    DEFINE_INSTRUCTION(stl,     0x17,  1,  0,  1,  0);
    DEFINE_INSTRUCTION(ldg2,    0x18,  2,  0,  0,  1); // global variable by pool ref
    DEFINE_INSTRUCTION(stg2,    0x19,  2,  0,  1,  0);
    DEFINE_INSTRUCTION(ldc2,    0x1A,  2,  0,  0,  1); // constant pool value
    DEFINE_INSTRUCTION(stc2,    0x1B,  2,  0,  0,  1); // only allowed during load time
    DEFINE_INSTRUCTION(ldf,     0x1C,  1,  0,  0,  1); // field 
    DEFINE_INSTRUCTION(stf,     0x1D,  1,  0,  1,  0);
    DEFINE_INSTRUCTION(ldfc2,   0x1E,  2,  0,  0,  1); // field by pool ref
    DEFINE_INSTRUCTION(stfc2,   0x1F,  2,  0,  1,  0);
    
    // arithmetic
    DEFINE_INSTRUCTION(iadd,    0x30,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(isub,    0x31,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(imul,    0x32,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(idiv,    0x33,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(imod,    0x34,  0,  0,  2,  1);
    
    DEFINE_INSTRUCTION(fadd,    0x35,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fsub,    0x36,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fmul,    0x37,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fdiv,    0x38,  0,  0,  2,  1);
    
    DEFINE_INSTRUCTION(inot,    0x39,  0,  0,  1,  1); // bitwise not
    DEFINE_INSTRUCTION(iand,    0x3A,  0,  0,  2,  1); // bitwise and
    DEFINE_INSTRUCTION(ior,     0x3B,  0,  0,  2,  1); // bitwise or
    DEFINE_INSTRUCTION(ixor,    0x3C,  0,  0,  2,  1); // bitwise or
    DEFINE_INSTRUCTION(ishl,    0x3D,  0,  0,  2,  1); // left shift
    DEFINE_INSTRUCTION(ishr,    0x3E,  0,  0,  2,  1); // right shift (signed)
    DEFINE_INSTRUCTION(ishru,   0x3F,  0,  0,  2,  1); // right shift (0s)
    DEFINE_INSTRUCTION(ilnot,   0x40,  0,  0,  1,  1); // logical not
    
    // TODO: handle: 2,4byte arithmetic,oveflows, sign extension, etc
    
    // comparison
    DEFINE_INSTRUCTION(igt,     0x50,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(ilt,     0x51,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(ige,     0x52,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(ile,     0x53,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(ieq,     0x54,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(ine,     0x55,  0,  0,  2,  1);
    
    DEFINE_INSTRUCTION(fgt,     0x56,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(flt,     0x57,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fge,     0x58,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fle,     0x59,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(feq,     0x5A,  0,  0,  2,  1);
    DEFINE_INSTRUCTION(fne,     0x5B,  0,  0,  2,  1);
    
    // control
    DEFINE_INSTRUCTION(brt2,    0x60,  2,  0,  1,  0);
    DEFINE_INSTRUCTION(brf2,    0x61,  2,  0,  1,  0);
    DEFINE_INSTRUCTION(bra2,    0x62,  2,  0,  0,  0);
    
    DEFINE_INSTRUCTION(call2,   0x70,  2,  0,  -1, 1);
    //DEFINE_INSTRUCTION(clos2,   0x71,  2,  0,   0, 1);
    DEFINE_INSTRUCTION(clos21,  0x72,  2,  1,  -1, 1);
    DEFINE_INSTRUCTION(app,     0x73,  1,  0,  -1, -1);
    DEFINE_INSTRUCTION(ret,     0x74,  0,  0,   1, 0);
    DEFINE_INSTRUCTION(retv,    0x75,  0,  0,   0, 0); // not used by dor
    
    // heap
    DEFINE_INSTRUCTION(chunk,   0x80,  1,  0,   -1, 1);
    DEFINE_INSTRUCTION(unchunk, 0x81,  1,  0,    1, -1);
    DEFINE_INSTRUCTION(alloc,   0x82,  0,  0,    1, 1);
    
    
    // special
    DEFINE_INSTRUCTION(sys,     0x90,  1,  0,    -1, -1);

    
    template<size_t N = 255>
    inline void initInstructions() {
        getInstr<N>();
        initInstructions<N-1>();
    }
    
    template<>
    inline void initInstructions<0>() {
        getInstr<0>();
    }
}
