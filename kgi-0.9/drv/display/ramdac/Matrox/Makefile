OBJECTS = Gx00.o

Gx00.o:	Gx00-meta.o Gx00-bind.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),Matrox)
default.entry:: $(CONFIG_RAMDAC_MODEL).o
endif
