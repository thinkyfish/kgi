OBJECTS = TNT2.o

TNT2.o:	TNT2-meta.o TNT2-bind.o ../IBM/VGA-text-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CHIPSET_VENDOR),nVidia)
default.entry:: $(CONFIG_CHIPSET_MODEL).o
endif
