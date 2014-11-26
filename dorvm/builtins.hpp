//to be included in interpreter.cpp



// arithmetic!!

#define DEFINE_BUILTIN_BINOP(func, field, op) \
obj::word_t func(obj::word_t *args) { \
    obj::word_t res; \
    res.field = args[0].field op args[1].field; \
    return res; \
}

DEFINE_BUILTIN_BINOP(addInt, i64, +);
DEFINE_BUILTIN_BINOP(subInt, i64, -);
DEFINE_BUILTIN_BINOP(mulInt, i64, *);
DEFINE_BUILTIN_BINOP(divInt, i64, /);
DEFINE_BUILTIN_BINOP(modInt, i64, %);

DEFINE_BUILTIN_BINOP(addFloat, f64, +);
DEFINE_BUILTIN_BINOP(subFloat, f64, -);
DEFINE_BUILTIN_BINOP(mulFloat, f64, *);
DEFINE_BUILTIN_BINOP(divFloat, f64, /);

DEFINE_BUILTIN_BINOP(ltInt, i64, <);
DEFINE_BUILTIN_BINOP(gtInt, i64, >);
DEFINE_BUILTIN_BINOP(leInt, i64, <=);
DEFINE_BUILTIN_BINOP(geInt, i64, >=);
DEFINE_BUILTIN_BINOP(eqInt, i64, ==);
DEFINE_BUILTIN_BINOP(neInt, i64, !=);

DEFINE_BUILTIN_BINOP(ltFloat, f64, <);
DEFINE_BUILTIN_BINOP(gtFloat, f64, >);
DEFINE_BUILTIN_BINOP(leFloat, f64, <=);
DEFINE_BUILTIN_BINOP(geFloat, f64, >=);
DEFINE_BUILTIN_BINOP(eqFloat, f64, ==);
DEFINE_BUILTIN_BINOP(neFloat, f64, !=);

DEFINE_BUILTIN_BINOP(and_, i64, &&);
DEFINE_BUILTIN_BINOP(or_, i64, ||);

obj::word_t not_(obj::word_t *args) {
    obj::word_t res;
    res.i64 = !args[0].i64;
    return res;
}

// io

obj::word_t error_(obj::word_t *args) {
    std::cerr << ((const char *)args[0].ptr + 8);
    assert(0);
    return wordPtr(0);
}

obj::word_t readInt(obj::word_t *args) {
    obj::word_t res;
    std::cin >> res.i64;
    return res;
}

obj::word_t readFloat(obj::word_t *args) {
    obj::word_t res;
    std::cin >> res.f64;
    return res;
}

obj::word_t readString(obj::word_t *args) {
    std::string s;
    std::cin >> s;
    
    obj::word_t res;
    res.ptr = makeString(s.data(), s.size());
    return res;
}

obj::word_t printInt(obj::word_t *args) {
    std::cout << args[0].i64;
    return wordPtr(0);
}

obj::word_t printFloat(obj::word_t *args) {
    std::cout << args[0].f64;
    return wordPtr(0);
}

obj::word_t printString(obj::word_t *args) {
    //std::cerr << "PRINTSTTR" << std::endl;
    std::cout << ((const char *)args[0].ptr + 8);
    return wordPtr(0);
}

obj::word_t intToFloat(obj::word_t *args) {
    obj::word_t res;
    res.f64 = (double) args[0].i64;
    return res;
}

obj::word_t floatToInt(obj::word_t *args) {
    obj::word_t res;
    res.i64 = (int64_t) args[0].f64;
    return res;
}


// arrays
// compact format for arrays of 2byte, 1byte, 4byte values??
obj::word_t Array(obj::word_t *args) {
    uint64_t size = args[0].i64;
    assert(size >= 0);

    obj::word_t *array = new obj::word_t[2 + size];
    array[0].i64 = size;

    for(size_t i = 1; i <= size; i++) {
        array[i] = args[1];
    }

    return wordPtr(array);
}

obj::word_t arrayGet(obj::word_t *args) {
    obj::word_t *array = (obj::word_t *) args[0].ptr;
    size_t size = array[0].i64;
    int64_t index = args[1].i64;
    assert(index >= 0 && index < size);
    
    return array[1 + index];
}

obj::word_t arraySet(obj::word_t *args) {
    obj::word_t *array = (obj::word_t *) args[0].ptr;
    size_t size = array[0].i64;
    int64_t index = args[1].i64;
    assert(index >= 0 && index < size);

    //std::cerr << "arrset sz " << size << " i " << index << std::endl;
    
    array[1 + index] = args[2];
    //std::cerr << "uccess rrset sz " << size << " i " << index << std::endl;
    
    return args[2];
}

obj::word_t arraySize(obj::word_t *args) {
    return ((obj::word_t *) args[0].ptr)[0];
}

void initBuiltins(obj::word_t *pool) {
    size_t i = 0;

    pool[i++].ptr = (void *) makeClosure(new Proc(addInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(subInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(mulInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(divInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(modInt, 2), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(addFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(subFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(mulFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(divFloat, 2), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(ltInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(gtInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(leInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(geInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(eqInt, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(neInt, 2), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(ltFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(gtFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(leFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(geFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(eqFloat, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(neFloat, 2), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(and_, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(or_, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(not_, 1), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(error_, 1), nullptr, 0);
    
    pool[i++].ptr = (void *) makeClosure(new Proc(printInt, 1), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(printFloat, 1), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(printString, 1), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(readInt, 1), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(readFloat, 1), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(readString, 1), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(intToFloat, 1), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(floatToInt, 1), nullptr, 0);

    pool[i++].ptr = (void *) makeClosure(new Proc(Array, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(arrayGet, 2), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(arraySet, 3), nullptr, 0);
    pool[i++].ptr = (void *) makeClosure(new Proc(arraySize, 1), nullptr, 0);


    


    
}
