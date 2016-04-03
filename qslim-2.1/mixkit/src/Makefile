#
# Makefile for building the MixKit library
# $Id: Makefile,v 1.50.2.1 2001/09/21 16:18:23 garland Exp $
#

include ../../libgfx/gfx-config

CORE_SRCS = mixmsg.cxx MxCmdParser.cxx MxAsp.cxx

MATH_SRCS = MxMat3-jacobi.cxx MxMat4-jacobi.cxx \
	    MxGeom3D.cxx MxTriProject.cxx MxFrame.cxx MxFitFrame.cxx \
	    MxFitFrame-2.cxx MxGeom2D.cxx mixmops.cxx

MODEL_SRCS = MxBlockModel.cxx MxStdModel.cxx MxSMF.cxx MxQMetric3.cxx \
             MxQSlim.cxx MxStdSlim.cxx MxQMetric.cxx MxPropSlim.cxx \
             MxDualModel.cxx MxFaceTree.cxx MxFaceTree-2.cxx MxDualSlim.cxx \
             MxEdgeFilter.cxx MxFeatureFilter.cxx

DATA_SRCS = MxHeap.cxx 

# These modules require OpenGL or Mesa
GL_SRCS = MxCamera.cxx MxQVis3.cxx \
          MxStdRender.cxx MxGLDebug.cxx MxGLUtils.cxx MxFitFrame-vis.cxx

# These modules require the FLTK toolkit <http://www.fltk.org/>
GUI_SRCS =  MxStdGUI.cxx

SRCS = $(CORE_SRCS) $(MATH_SRCS) $(MODEL_SRCS) $(DATA_SRCS) \
       $(GL_SRCS) $(GUI_SRCS)

OBJS = $(SRCS:.cxx=.o)


SRCSALL = mixmsg.cxx MxCmdParser.cxx MxAsp.cxx MxMat3-jacobi.cxx MxMat4-jacobi.cxx MxGeom3D.cxx MxTriProject.cxx MxFrame.cxx MxFitFrame.cxx MxFitFrame-2.cxx MxGeom2D.cxx mixmops.cxx MxBlockModel.cxx MxStdModel.cxx MxSMF.cxx MxQMetric3.cxx MxQSlim.cxx MxStdSlim.cxx MxQMetric.cxx MxPropSlim.cxx MxDualModel.cxx MxFaceTree.cxx MxFaceTree-2.cxx MxDualSlim.cxx MxEdgeFilter.cxx MxFeatureFilter.cxx MxHeap.cxx
OBJALL = mixmsg.o MxCmdParser.o MxAsp.o MxMat3-jacobi.o MxMat4-jacobi.o MxGeom3D.o MxTriProject.o MxFrame.o MxFitFrame.o MxFitFrame-2.o MxGeom2D.o mixmops.o MxBlockModel.o MxStdModel.o MxSMF.o MxQMetric3.o MxQSlim.o MxStdSlim.o MxQMetric.o MxPropSlim.o MxDualModel.o MxFaceTree.o MxFaceTree-2.o MxDualSlim.o MxEdgeFilter.o MxFeatureFilter.o MxHeap.o

libmix.a: foo
	$(AR) cru libmix.a $(OBJALL) 
	$(RANLIB) libmix.a

foo:
	for src in $(SRCSALL); do \
	$(CC) -c -fPIC $$src $(LDFLAGS) $(CXXFLAGS) $(LIBMIX) -lm; \
	done
#############################################################


clean:
	-$(RM) -f $(OBJALL) libmix.a

depend:
	$(CXX_DEPEND) $(SRCS) > Makefile.dep


-include Makefile.dep
#sinclude Makefile.dep     # Alternative syntax for some makes
