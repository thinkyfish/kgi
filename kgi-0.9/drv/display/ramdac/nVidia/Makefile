OBJECTS = TNT2.o

TNT2.o:	TNT2-meta.o TNT2-bind.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),nVidia)
default.entry: $(CONFIG_RAMDAC_MODEL).o
else
default.entry:
endif
