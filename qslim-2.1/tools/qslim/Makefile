include ../../mixkit/mix-config

PG=-I/usr/include/postgresql/9.4/server
DEST=/usr/lib/postgresql/9.4/lib/
SRCS = main.cxx singleQ.cxx  externQslim.cxx
OBJS = $(SRCS:.cxx=.o)
CXXFLAGS += -fPIC
CFLAGS += -fPIC

# qqq: $(OBJS)
# 	gcc -fPIC -c qqqq.c
# 	g++ -o qqq qqqq.o $(OBJS) $(LDFLAGS) $(LIBMIX) -lm

qslim.so: $(OBJS)  ../../mixkit/src/libmix.a ../../libgfx/src/libgfx.a
	$(CXX) $(PG) -fPIC -c qslim.c
	$(CXX) -shared -o $(DEST)qslim.so qslim.o $(OBJS) $(LDFLAGS) $(LIBMIX) -lm
#strip --strip-all $(DEST)qslim.so
#strip --strip-unneeded $(DEST)qslim.so
main.o : main.cxx 
	$(CC) -c main.cxx $(LDFLAGS) $(CXXFLAGS) $(LIBMIX) -lm

singleQ.o : singleQ.cxx
	$(CC) -c singleQ.cxx $(LDFLAGS) $(CXXFLAGS) $(LIBMIX) -lm

exernQslim.o : exernQslim.cxx
	$(CC) -c exernQslim.cxx $(LDFLAGS) $(CXXFLAGS) $(LIBMIX) -lm

clean:
	rm -f $(OBJS) $(DEST)qslim.so qslim.o qqqq.o qqq *.o


