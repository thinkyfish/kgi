OBJECTS = MDA.o VGA.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),IBM)
default.entry: $(CONFIG_CLOCK_MODEL).o
else
default.entry:
endif
