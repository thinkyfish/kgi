OBJECTS = Gx00.o

Gx00.o:	Gx00-meta.o Gx00-bind.o ../pll-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),Matrox)
default.entry:: $(CONFIG_CLOCK_MODEL).o
endif
