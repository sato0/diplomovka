CXX=gcc
CXXFLAGS=-I/usr/include/postgresql/9.4/server
DEST=/usr/lib/postgresql/9.4/lib/
all: merge2filesPG.o addonePG.o

merge2filesPG:
	$(CXX) $(CXXFLAGS) -fpic -c merge2filesPG.c
	$(CXX) -shared -o $(DEST)merge2filesPG.so merge2filesPG.o

addonePG.o:
	$(CXX) $(CXXFLAGS) -fpic -c addonePG.c
	$(CXX) -shared -o $(DEST)addonePG.so addonePG.o

clean:
	rm -f merge2filesPG.o $(DEST)merge2filesPG.so addonePG.o $(DEST)addonePG.so