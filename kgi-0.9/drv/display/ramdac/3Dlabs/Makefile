OBJECTS = PERMEDIA2.o PERMEDIA2v.o

PERMEDIA2.o:	PERMEDIA2-meta.o PERMEDIA2-bind.o

PERMEDIA2v.o:	PERMEDIA2v-meta.o PERMEDIA2v-bind.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),3Dlabs)
default.entry: $(CONFIG_RAMDAC_MODEL).o
else
default.entry:
endif
