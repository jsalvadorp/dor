#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "heap.h"

word_t * makechunk(nint words) {
    // GARBAGE COLLECTION!!!!!!!!!!!!!1
    word_t * chunk = ALLOC((1 + words) * sizeof(word_t));
    chunk[0].i = words;
    //fprintf(stderr, "made chunk\n");
    return chunk;
}

// ocaml strings:
// __ __ __ 00
// __ __ 00 01
// __ 00 00 02
// 00 00 00 03
// or up to 07 in 64bit

// int2 array:
// __ __ 00 00
// 00 00 [00 01]

// or up to 03 in 64bit


// int4 array:
// no padding
// or in 64bit
// __ __ __ __ 00 00 00 00
// 00 00 00 00 [00 00 00 01]

// the last element holds the padcount-1
// (in machine form. with native endianness)

// cities in dust cocteau twins

word_t * makearr1(nint elc) { // elsize = 1, 2, 4, 8
    nint words = elc / sizeof(word_t) + 1; 
    
    word_t * arr = makechunk(words);
    arr[words].i = 0;
    char * arr1 = (char *) (arr + words + 1); // end of the array
    *(arr1 - 1) = (words * sizeof(word_t) - elc - 1); // padcount-1
    
    return arr;
}

word_t * makearrs(const char * s) { // elsize = 1, 2, 4, 8
    nint len = strlen(s);
    
    word_t * a = makearr1(len);
    memcpy(a + 1, s, len);
    
    return a;
}

word_t * makearr2(nint elc) { // elsize = 1, 2, 4, 8
    nint words = elc / (sizeof(word_t)/sizeof(int16_t)) + 1; 
    
    word_t * arr = makechunk(words);
    arr[words].i = 0;
    int16_t * arr1 = (int16_t *) (arr + words + 1); // end of the array
    *(arr1 - 1) = (words * (sizeof(word_t)/sizeof(int16_t)) - elc - 1); // padcount-1
    
    return arr;
}

word_t * makearr4(nint elc) { // elsize = 1, 2, 4, 8
#if ARCH64
    nint words = elc / (sizeof(word_t)/sizeof(int32_t)) + 1; 
    
    word_t * arr = makechunk(words);
    arr[words].i = 0;
    int32_t * arr1 = (int32_t *) (arr + words + 1); // end of the array
    *(arr1 - 1) = (words * (sizeof(word_t)/sizeof(int32_t)) - elc - 1); // padcount-1
    
    return arr;
#else
    return makechunk(elc);
#endif
}

word_t * makearr8(nint elc) { // elsize = 1, 2, 4, 8
#if ARCH64
    return makechunk(elc);
#else
    return makechunk(elc * 2);
#endif
}


void dumpchunk(const word_t * chunk) {
    int x = chunk[0].i;
    //fprintf(stderr, "(%6d) ", x);
    const char * data = (const char *)(chunk + 1);
    for(int i = 0; i < x; i++) {
        //fprintf(stderr, "| ");
        for(int j = 0; j < sizeof(word_t); j++) {
            //fprintf(stderr, "%2x ", data[i * sizeof(word_t) + j]);
        }
        
    }
    //puts("");
}

#if 0




int main() {
    word_t * a = makearrs("a"), 
    *aa = makearrs("aa"), 
    *aaa = makearrs("aaa"), 
    *aaaa = makearrs("aaaa"),
    *aaaa_a = makearrs("aaaaa"), 
    *aaaa_aa = makearrs("aaaaaa"), 
    *aaaa_aaa = makearrs("aaaaaaa"), 
    *aaaa_aaaa = makearrs("aaaaaaaa"), 
    *aaaa_aaaa_a = makearrs("aaaaaaaaa");
    
    dumpchunk(a);
    dumpchunk(aa);
    dumpchunk(aaa);
    dumpchunk(aaaa);
    
    dumpchunk(aaaa_a);
    dumpchunk(aaaa_aa);
    dumpchunk(aaaa_aaa);
    dumpchunk(aaaa_aaaa);
    
    dumpchunk(aaaa_aaaa_a);
    
    word_t * e0 = makearr2(0),  
    * e1 = makearr2(1),  
    * e2 = makearr2(2),  
    * e3 = makearr2(3),  
    * e4 = makearr2(4),  
    * e5 = makearr2(5);
    
    int16_t
    *s0 = (int16_t *) (e0 + 1),
    *s1 = (int16_t *) (e1 + 1),
    *s2 = (int16_t *) (e2 + 1),
    *s3 = (int16_t *) (e3 + 1),
    *s4 = (int16_t *) (e4 + 1),
    *s5 = (int16_t *) (e5 + 1);
    
    printf("-- %p %p\n", e3, s3);
    
    
    for(int i = 0; i < 0; i++)
        s0[i] = 3;
    for(int i = 0; i < 1; i++)
        s1[i] = 3;
    for(int i = 0; i < 2; i++)
        s2[i] = 3;
    for(int i = 0; i < 3; i++)
        s3[i] = 3;
    for(int i = 0; i < 4; i++)
        s4[i] = 3;
    for(int i = 0; i < 5; i++)
        s5[i] = 3;
        
        
    printf("\n\n");
        
    dumpchunk(e0);
    dumpchunk(e1);
    dumpchunk(e2);
    dumpchunk(e3);
    dumpchunk(e4);
    dumpchunk(e5);
    
    word_t * i0 = makearr4(0),  
    * i1 = makearr4(1),  
    * i2 = makearr4(2),  
    * i3 = makearr4(3);
    
    int32_t
    *u0 = (int32_t *) (i0 + 1),
    *u1 = (int32_t *) (i1 + 1),
    *u2 = (int32_t *) (i2 + 1),
    *u3 = (int32_t *) (i3 + 1);
    
    printf("\n\n");
    
    for(int i = 0; i < 0; i++)
        u0[i] = 3;
    for(int i = 0; i < 1; i++)
        u1[i] = 3;
    for(int i = 0; i < 2; i++)
        u2[i] = 3;
    for(int i = 0; i < 3; i++)
        u3[i] = 3;
        
    dumpchunk(i0);
    dumpchunk(i1);
    dumpchunk(i2);
    dumpchunk(i3);
}

#endif

