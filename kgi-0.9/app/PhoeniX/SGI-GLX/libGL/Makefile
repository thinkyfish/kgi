# -----------------------------------------------------------------------------
#	SGI OpenGL Sample Implementation -- GL library
# -----------------------------------------------------------------------------

default.exit:: libGL.so

# Remove -D__GL_SUPPORT_ARB_IMAGING_SUBSET if no ARB_imaging features
# are wanted.

CC_OPT += -D_LANGUAGE_C -Dunix 

CC_LIBS = Xext

libGL.so:		\
	libGL.o 	\
	clientattrib.o	\
	compsize.o	\
	eval.o		\
	glxcmds.o	\
	glxext.o	\
	pixel.o		\
	pixelstore.o	\
	render2.o	\
	renderpix.o	\
	single2.o	\
	singlepix.o	\
	vertarr.o	\
	g_render.o	\
	g_single.o	\
	g_vendpriv.o

