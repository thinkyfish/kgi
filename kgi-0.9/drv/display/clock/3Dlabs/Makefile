OBJECTS = PERMEDIA2.o PERMEDIA2v.o

PERMEDIA2.o:	PERMEDIA2-meta.o PERMEDIA2-bind.o ../pll-meta.o

PERMEDIA2v.o:	PERMEDIA2v-meta.o PERMEDIA2v-bind.o ../pll-meta.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),3Dlabs)
default.entry: $(CONFIG_CLOCK_MODEL).o
else
default.entry:
endif
