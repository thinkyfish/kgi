OBJECTS = TVP3026.o

TVP3026.o:	TVP3026-meta.o TVP3026-bind.o ../pll-meta.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),TI)
default.entry: $(CONFIG_CLOCK_MODEL).o
else
default.entry:
endif
