OBJECTS = Gx00.o

Gx00.o: Gx00-meta.o Gx00-bind.o
#../IBM/VGA-text-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CHIPSET_VENDOR),Matrox)
default.entry:: $(CONFIG_CHIPSET_MODEL).o
endif
