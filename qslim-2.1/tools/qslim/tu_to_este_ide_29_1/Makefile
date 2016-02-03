include ../../mixkit/mix-config

SRCS = main.cxx qslim.cxx
OBJS = $(SRCS:.cxx=.o)

all: qslim 

qslim : $(OBJS) ../../mixkit/src/libmix.a
	$(CXX) -o qslim $(OBJS) $(LDFLAGS) $(LIBMIX) -lm

clean:
	-$(RM) -f $(OBJS) qslim
