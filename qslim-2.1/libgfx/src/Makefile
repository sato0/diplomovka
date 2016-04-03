include ../gfx-config

VEC_SRCS = mat2.cxx mat3.cxx mat4.cxx quat.cxx symmat3.cxx
GEO_SRCS = geom3d.cxx
IMG_SRCS = raster.cxx raster-pnm.cxx raster-tiff.cxx raster-jpeg.cxx \
		raster-png.cxx
MISC_SRCS = time.cxx script.cxx
GUI_SRCS = gui.cxx gltools.cxx baseball.cxx trackball.cxx arcball.cxx

SRCS = $(VEC_SRCS) $(GEO_SRCS) $(IMG_SRCS) $(MISC_SRCS) $(GUI_SRCS)
OBJS = $(SRCS:.cxx=.o)

SRCSALL = mat2.cxx mat3.cxx mat4.cxx quat.cxx symmat3.cxx geom3d.cxx raster.cxx raster-pnm.cxx raster-tiff.cxx raster-jpeg.cxx raster-png.cxx time.cxx script.cxx
OBJSALL = $(SRCSALL:.cxx=.o)
libgfx.a: $(OBJSALL) foo
	$(AR) cru libgfx.a $(OBJSALL)
	$(RANLIB) libgfx.a

foo:
	for src in $(SRCSALL); do \
	$(CC) -c -fPIC $$src $(LDFLAGS) $(CXXFLAGS) -lm; \
	done


tags: $(SRCSALL) ../include/gfx/*.h
	ctags $(SRCSALL) ../include/gfx/*.h

clean:
	-$(RM) $(OBJSALL) libgfx.a

depend:
	$(CXX_DEPEND) $(SRCSALL) > Makefile.dep

-include Makefile.dep
