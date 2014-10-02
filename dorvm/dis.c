// gcc vm.c -std=c99

#include <stdio.h>

typedef int word_t;

#include "opcodes.h"

#define READI32 \
    ((getc(in) << 24) | (getc(in) << 16) | (getc(in) << 8) | getc(in))
#define READI16 \
    ((getc(in) << 8) | getc(in))
    
#define GETI32(src, pos) \
    ((src[pos] << 24) | (src[pos + 1] << 16) | (src[pos + 2] << 8) | src[pos + 3])


int pool[65536];
int poolt[65536];
unsigned char procmem[65536];
unsigned char rawmem[65536];

int main(int argc, char *argv[]) {
    
    
    FILE *in = argc >= 2 ? fopen(argv[1], "r") : stdin;
    
    int poolsize = READI32;
    int procsize = READI32;
    int rawsize = READI32;
    int entry = READI32;
    
    printf("Pool size: %d\n", poolsize);
    
    unsigned char * code;
    
    for(int i = 0; i < poolsize; i++) {
        poolt[i] = getc(in);
        pool[i] = READI32;
    }
    
    for(int i = 0; i < procsize; i++)
        procmem[i] = getc(in);
    
    for(int i = 0; i < rawsize; i++)
        rawmem[i] = getc(in);
        
    
    
    for(int i = 0; i < poolsize; i++) {
        printf("%5d: ", i);
        if(poolt[i] == 'p') {
            printf("proc %d %s\n", pool[i], i == entry ? "<entry>" : "");
            
            unsigned char * code = procmem + pool[i];
            unsigned int nargs = GETI32(code, P_NARGS); \
            int nlocs = GETI32(code, P_NLOCS); \
            unsigned int psize = GETI32(code, P_PSIZE); \
            code += PROCSTART;
            
            printf("       args: %d\n", nargs);
            printf("       locs: %d\n", nlocs);
            printf("       code(%u): \n", psize);
            
            for(int j = 0; j < psize;) {
                int ipos = j;
                int opc = code[j++], arg = 0;
                for(int i = 0; i < o_size[opc]; i++) arg = (arg << 8) | code[j++];
                
                printf("          i%4d: %s ", ipos, o_names[opc]);
                
                if(o_size[opc]) {
                    printf("%d", arg);
                }
                puts("");
            }
        } else printf("%d\n", pool[i]);
    }
}
  
