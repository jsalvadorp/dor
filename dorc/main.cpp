#include <getopt.h>

#include "parser.hpp"
#include "analysis.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "codegen.hpp"


int main(int argc, char **argv) {
    static option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"assembly", no_argument, 0, 'a'},
    };

    string out_filename;
    
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


    initLexer(input_filenames[0]);
	parser::initParser();
	
	Ptr<List> tree = asList(parser::parseGroup(true));
    
    std::cout << "Syntax Tree Dump-----------------------" << std::endl;
    tree->dump(0);
    
    Ptr<Globals> globals = newPtr<Globals>();
    initTypes(globals);
    initGlobals(globals);
    
    Ptr<Sequence> unit = program(globals, tree);
    globals->assignIds();
    
    std::cout << "Annotated Tree Dump--------------------" << std::endl;
    for(Binding *b : globals->globals) {
        //std::cout << "|" <<  b->id << "|" << b->isconstexpr << "|" << b->constructor << std::endl;
        
        if(b->constructor) {
            std::cout << b->name << "{" << b->id << "} : ";
            b->type->dump();
            std::cout << std::endl;
        } else if(b->isconstexpr) {
            assert(b->defined);
            
            std::cout << b->name.str() << "{" << b->id << "} : ";
            b->type->dump(); 
            std::cout << " = " << std::endl;
            b->value->dump(1);
        }
    }
    
    unit->dump(0);
    std::cout << endl;
    
    std::cout << "Assembly output------------------------" << std::endl;
    initCompiler();
    compile(globals, unit);

    closeLexer();
    
    std::ofstream out("a.out", std::ios::out | std::ios::binary); 

    makeBinary(out);

}
