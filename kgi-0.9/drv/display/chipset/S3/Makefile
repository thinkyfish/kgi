OBJECTS = ViRGE.o

ViRGE.o: ViRGE-meta.o ViRGE-bind.o ../IBM/VGA-text-meta.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CHIPSET_VENDOR),S3)
default.entry: $(CONFIG_CHIPSET_MODEL).o
else
default.entry:
endif
