
default.exit:: phoenix

PHOENIX_OBJECTS = lib.o os.o dix.o ddx.o mi.o
PHOENIX_DRIVERS = ddx/cfb.o ddx/cfb08.o ddx/mfb.o ddx/xaa.o

SYSTEM_LIBS = -lm
SYSTEM_DIRS = 

BUILD_DIRS = -L$(DIR_TOP_BUILD)/lib
BUILD_LIBS = 

X11_LIBS = -lXfont -lX11 -lz -lXext
X11_DIRS = -L$(X11R6_DIR)/lib

#
#	SGI OpenGL Sample Implementation
#
BUILD_LIBS += -lGL -lGLX
PHOENIX_OBJECTS += SGI-GLX/ddx/glx.o
PHOENIX_OBJECTS += SGI-GLX/dix/glx.o



PHOENIX_DIRS = $(SYSTEM_DIRS) $(BUILD_DIRS) $(X11_DIRS)
PHOENIX_LIBS = $(SYSTEM_LIBS) $(BUILD_LIBS) $(X11_LIBS)

phoenix: $(PHOENIX_OBJECTS) $(PHOENIX_DRIVERS)
	$(CC) $^ $(PHOENIX_DIRS) $(PHOENIX_LIBS) -o $@
