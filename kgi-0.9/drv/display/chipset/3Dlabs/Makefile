OBJECTS = PERMEDIA.o

PERMEDIA.o:	PERMEDIA-meta.o PERMEDIA-bind.o ../IBM/VGA-text-meta.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CHIPSET_VENDOR),3Dlabs)
default.entry: $(CONFIG_CHIPSET_MODEL).o
else
default.entry:
endif
