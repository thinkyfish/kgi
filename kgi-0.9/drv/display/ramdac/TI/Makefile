OBJECTS = TVP3026.o

TVP3026.o:	TVP3026-meta.o TVP3026-bind.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),TI)
default.entry:: $(CONFIG_RAMDAC_MODEL).o
endif
