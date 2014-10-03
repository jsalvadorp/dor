CC=g++ -std=c++11
CFLAGS=-c
DFLAGS=-g
LFLAGS=-lfl

compiler_debug: lexer_src
	$(CC) $(LFLAGS) $(DFLAGS) dorc/types.cpp dorc/analysis.cpp dorc/parser.cpp dorc/symbol.cpp dorc/ast.cpp dorc/lexer.cpp -o bin/dorc
	
lexer_src:
	flex -o dorc/lexer.cpp dorc/lexer.l

clean:
	rm -rf *.o dorc dorc/lexer.cpp
