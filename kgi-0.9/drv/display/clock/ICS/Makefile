OBJECTS = GenDAC.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),ICS)
default.entry:: $(CONFIG_CLOCK_MODEL).o
endif
