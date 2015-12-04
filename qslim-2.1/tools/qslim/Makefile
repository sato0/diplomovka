include ../../mixkit/mix-config

SRCS = main.cxx qslim.cxx
OBJS = $(SRCS:.cxx=.o)
QVIS_OBJS = $(QVIS_SRCS:.cxx=.o)

all: qslim 

qslim : $(OBJS) ../../mixkit/src/libmix.a
	$(CXX) -o qslim $(OBJS) $(LDFLAGS) $(LIBMIX) -lm

clean:
	-$(RM) -f $(OBJS) qslim
	-cat /dev/null > Makefile.dep


depend:
	$(CXX_DEPEND) $(SRCS) $(QVIS_SRCS) > Makefile.dep

-include Makefile.dep
