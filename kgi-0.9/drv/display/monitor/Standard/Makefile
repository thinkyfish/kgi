OBJECTS = MDA.o SVGA.o VGA.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_MONITOR_VENDOR),Standard)
default.entry: $(CONFIG_MONITOR_MODEL).o
endif
