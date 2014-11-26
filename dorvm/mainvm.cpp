#include <iostream>
#include <getopt.h>
#include <vector>
#include "heap.hpp"
#include "interpreter.hpp"
#include "../dorc/instructions.hpp"

int main(int argc, char **argv) {
    instructions::initInstructions(); 
    static option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"assembly", no_argument, 0, 'a'},
    };

    std::string out_filename;
    
    int c;
    
    while(1) {
        int option_index = 0;

        c = getopt_long (argc, argv, "o:a",
                         long_options, &option_index);

        if(c == -1) break;

        /*switch(c) {
        case 'o':
            out_filename = optarg
        case '?':
        case 0:
            
        }*/
    }

    std::vector<std::string> input_filenames;

    while(optind < argc) {
        input_filenames.push_back(argv[optind++]);
    }
    
    std::ifstream bin(input_filenames[0].c_str(), std::ios::binary);
    Pool module = loadBinary(bin);
    
    size_t stack_size = 1024;
    obj::word_t *stack = new obj::word_t[stack_size];

    Proc *load = CLOS_PROC((obj::word_t *)(module.pool[module.load].ptr)),
         *entry = module.entry >= 0 ? (Proc *) CLOS_PROC((obj::word_t *)(module.pool[module.entry].ptr)) : nullptr;
    
    initBuiltins(module.pool);

    run(stack, stack_size, load);
    
    if(module.entry >= 0) {
        stack[stack_size - 1].ui64 = 0;
        run(stack, stack_size, entry);
    }
}
