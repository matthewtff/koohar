CC=g++
#CC=clang++

CFLAGS=-c -std=c++0x -g -ggdb -fPIC -D_DEBUG
#CFLAGS=-c -std=c++0x -O2 -fPIC

WARNINGS=-Wall -Wextra -Woverloaded-virtual
LIBS=-static -lpthread -lboost_regex
LIBNAME=libkoohar.so
MAJOR_VER=1
MINOR_VER=0
LIBFNAME=$(LIBNAME)
SOURCES=async.cc app.cc server.cc request.cc response.cc socket.cc sender.cc\
file.cc filemapping.cc thread.cc date.cc xmlobjects.cc parser.cc webpage.cc\
webui.cc http_parser.cc uri_parser.cc
OBJECTS=$(SOURCES:.cc=.o)

all: static shared

shared: $(OBJECTS)
	$(CC) $(WARNINGS) $(LIBS) -shared -Wl,-soname,$(LIBNAME) -o $(LIBFNAME) *.o

static: $(OBJECTS)
	ar crf libkoohar.a *.o

.cc.o:
	$(CC) $(WARNINGS) $(CFLAGS) $< -o $@

rebuild: clean all

clean:
	rm *.o libkoohar*
