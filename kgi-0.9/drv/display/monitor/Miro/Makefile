OBJECTS = miro2PAGE.o miroC2071.o miroD1785TE.o miroD2085TE.o \
	miroTRINITRON2.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_MONITOR_VENDOR),Miro)
default.entry: $(CONFIG_MONITOR_MODEL).o
endif
