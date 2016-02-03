include ../../mixkit/mix-config

SRCS = main.cxx singleQ.cxx 
OBJS = $(SRCS:.cxx=.o)
CSOURCE = qslim.c 
OBJSC = $(CSOURCE:.c=.o)

all: qslim 

qslim : $(OBJS) $(OBJSC) ../../mixkit/src/libmix.a
	$(CXX) -o qslim $(OBJS) qslim.o $(LDFLAGS) $(LIBMIX) -lm

qslim.o: qslim.c 
	$(CXX) -c -o qslim.o qslim.c 

clean:
	-$(RM) -f $(OBJS) qslim.o qslim
