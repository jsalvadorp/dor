CC=g++ -std=c++11 -Wno-deprecated-register
CCFLAGS=-c
DFLAGS=-g
LDFLAGS=
COMPILER=bin/dorc

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS +=-D LINUX
	LDFLAGS +=-lfl
endif
ifeq ($(UNAME_S),Darwin)
	CCFLAGS +=-D OSX
	LDFLAGS +=-ll
endif

$(COMPILER): obj/main.o obj/types.o obj/analysis.o obj/parser.o obj/symbol.o obj/ast.o obj/lexer.o obj/codegen.o
	mkdir -p bin
	$(CC) $(LDFLAGS) $(DFLAGS) $^ -o $@

obj/main.o: dorc/main.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/types.o: dorc/types.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/analysis.o: dorc/analysis.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/parser.o: dorc/parser.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/symbol.o: dorc/symbol.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/ast.o: dorc/ast.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/lexer.o: obj/lexer.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
obj/codegen.o: dorc/codegen.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@
	
	
obj/lexer.cpp: dorc/lexer.l
	mkdir -p obj
	flex -o $@ $^ 

clean:
	rm obj/* bin/dorc
