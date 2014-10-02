CC=g++ -std=c++11
CFLAGS=-c
DFLAGS=-g
LFLAGS=-lfl

debug: lexer_src
	$(CC) $(LFLAGS) $(DFLAGS) parser.cpp symbol.cpp ast.cpp lexer.cpp -o ../bin/dorc
	
lexer_src:
	flex -o lexer.cpp lexer.l

clean:
	rm -rf *.o dorc lexer.cpp
