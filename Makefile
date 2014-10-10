CC=g++ -std=c++11 -Wno-deprecated-register
CCFLAGS=-c
DFLAGS=-g
LDFLAGS=

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS +=-D LINUX
	LDFLAGS +=-lfl
endif
ifeq ($(UNAME_S),Darwin)
	CCFLAGS +=-D OSX
	LDFLAGS +=-ll
endif

compiler_debug: lexer_src
	mkdir -p bin
	$(CC) $(LDFLAGS) $(DFLAGS) dorc/main.cpp dorc/types.cpp dorc/analysis.cpp dorc/parser.cpp dorc/symbol.cpp dorc/ast.cpp dorc/lexer.cpp -o bin/dorc
	
lexer_src:
	flex -o dorc/lexer.cpp dorc/lexer.l
	

clean:
	rm -rf *.o dorc dorc/lexer.cpp
