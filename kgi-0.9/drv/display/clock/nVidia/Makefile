OBJECTS = TNT2.o

TNT2.o:	TNT2-meta.o TNT2-bind.o ../pll-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),nVidia)
default.entry:: $(CONFIG_CLOCK_MODEL).o
endif
