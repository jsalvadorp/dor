#ifndef HEAP_H
#define HEAP_H

#define CHUNKOFFS 1
#define CHUNKSIZE(ch) ch[0].i

#define ALLOC(x) malloc(x)

#define ARCH64 1

#if ARCH64
	typedef long long nint;
#else
	typedef int nint;
#endif

typedef short int16_t;
typedef int int32_t;

typedef union word_t {
	nint i;
	void * p;
} word_t;

word_t * makechunk(nint words);
word_t * makearr1(nint elc);
word_t * makearrs(const char * s);
word_t * makearr2(nint elc);
word_t * makearr4(nint elc);
word_t * makearr8(nint elc);

void dumpchunk(const word_t * chunk);


#endif
