
default.exit:: phoenix

SYSTEM_LIB = -lm
SYSTEM_DIR = 

X11_LIB = -lXfont -lX11 -lz
X11_DIR = -L$(X11R6_DIR)/lib


LIB = $(SYSTEM_LIB) $(X11_LIB)
DIR = $(SYSTEM_DIR) $(X11_DIR)

phoenix: lib.o os.o dix.o ddx.o mi.o ddx/cfb.o ddx/cfb08.o ddx/mfb.o ddx/xaa.o
	$(CC) $^ $(DIR) $(LIB) -o $@
