OBJECTS = IDAC.o ViRGE.o

IDAC.o:		IDAC-meta.o IDAC-bind.o ../pll-meta.o

ViRGE.o:	ViRGE-meta.o ViRGE-bind.o ../pll-meta.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),S3)
default.entry:: $(CONFIG_CLOCK_MODEL).o
endif
