OBJECTS = ViRGE.o

ViRGE.o:	ViRGE-meta.o ViRGE-bind.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_RAMDAC_VENDOR),S3)
default.entry: $(CONFIG_RAMDAC_MODEL).o
else
default.entry:
endif
