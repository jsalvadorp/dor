#include "parser.hpp"
#include "analysis.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "codegen.hpp"


int main() {
    initLexer();
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
}
