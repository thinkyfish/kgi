OBJECTS = VGA.o

VGA.o:	VGA-meta.o VGA-bind.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),IBM)
default.entry:: $(CONFIG_RAMDAC_MODEL).o
endif
