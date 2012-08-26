#CC=g++
CC=clang++

#CFLAGS=-c -std=c++0x -g -ggdb -stdlib=libc++ -D_DEBUG
CFLAGS=-c -std=c++0x -O2 -stdlib=libc++ -g

WARNINGS=-Wall -Wextra -Wold-style-cast -Woverloaded-virtual
LIBNAME=koohar
LIBPATH=.
INCPATH=../src/
INC=-I$(INCPATH)

all: tests

tests: main.o index.o loginbox.o
	$(CC) -g -O2 -std=c++0x -stdlib=libc++ $(WARNINGS) $(INC) main.o index.o loginbox.o -lboost_regex -lpthread -L$(LIBPATH) -l$(LIBNAME) -o koohar

main.o: main.cc index.hh
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) main.cc

index.o: index.cc index.hh loginbox.hh
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) index.cc

loginbox.o: loginbox.cc loginbox.cc
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) loginbox.cc

clean:
	rm -rf *.o koohar
