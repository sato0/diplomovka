CXX=gcc
CXXFLAGS=-I/usr/include/postgresql/9.4/server
all: pg3angles-1.01.o

pg3angles-1.01.o:
	$(CXX) $(CXXFLAGS) -fpic -c pg3angles-1.01.c
	$(CXX) -shared -o pg3angles-1.01.so pg3angles-1.01.o

clean:
	rm -f pg3angles-1.01.o pg3angles-1.01.so