OBJECTS = AV9064.o AV9194_04.o AV9194_07.o AV9194_11.o AV9194_12.o \
	AV9194_36.o AV9194_37.o AV9194_42.o AV9194_56.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_CLOCK_VENDOR),Avasem)
default.entry: (CONFIG_CLOCK_MODEL).o
else
default.entry:
endif
