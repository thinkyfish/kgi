OBJECTS = MicroScan6G.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_MONITOR_VENDOR),ADI)
default.entry:: $(CONFIG_MONITOR_MODEL).o
endif
