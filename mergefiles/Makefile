CXX=gcc
CXXFLAGS=-I/usr/include/postgresql/9.4/server

all: merge2files

merge2files:
	$(CXX) $(CXXFLAGS) -fpic -c merge2files.c
	$(CXX) -o merge2files merge2files.o

clean:
	rm -f merge2files.o merge2files