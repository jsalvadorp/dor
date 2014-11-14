CC=g++ -std=c++11 
CCFLAGS=-c
DFLAGS=-g
LDFLAGS=
VM=bin/dorvm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS +=-D LINUX
endif
ifeq ($(UNAME_S),Darwin)
	CCFLAGS +=-D OSX
endif

$(VM): obj/loader.o obj/mainvm.o obj/interpreter.o obj/heap.o
	mkdir -p bin
	$(CC) $(LDFLAGS) $(DFLAGS) $^ -o $@

obj/loader.o: loader.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@

obj/mainvm.o: mainvm.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@

obj/interpreter.o: interpreter.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@

obj/heap.o: heap.cpp
	mkdir -p obj
	$(CC) $(CCFLAGS) $(DFLAGS) $^ -o $@

clean:
	rm obj/* bin/*
