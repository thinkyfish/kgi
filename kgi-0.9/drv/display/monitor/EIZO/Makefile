OBJECTS = FlexScanF55.o FlexScanF56.o FlexScanF77.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_MONITOR_VENDOR),EIZO)
default.entry: $(CONFIG_MONITOR_MODEL).o
endif
