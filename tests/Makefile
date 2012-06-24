CC=g++
WARNINGS=-Wall -Wextra -Wold-style-cast -Woverloaded-virtual
CFLAGS=-c -std=c++0x -g -ggdb
#CFLAGS=-c -std=c++0x -O2
LIBNAME=koohar
LIBPATH=.
INCPATH=../src/
INC=-I$(INCPATH)

all: tests

tests: main.o index.o loginbox.o
	$(CC) -g -ggdb $(WARNINGS) main.o index.o loginbox.o -o koohar -L$(LIBPATH) -l$(LIBNAME) -lpthread -I$(INCPATH)

main.o: main.cc index.hh
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) main.cc

index.o: index.cc index.hh loginbox.hh
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) index.cc

loginbox.o: loginbox.cc loginbox.cc
	$(CC) $(WARNINGS) $(CFLAGS) $(INC) loginbox.cc

clean:
	rm -rf *.o koohar
