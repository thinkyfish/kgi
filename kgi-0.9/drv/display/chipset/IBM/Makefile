OBJECTS = VGA.o

VGA.o:	VGA-meta.o VGA-bind.o VGA-text-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CHIPSET_VENDOR),IBM)
default.entry:: $(CONFIG_CHIPSET_MODEL).o
endif
