OBJECTS = Belinea101510.o Belinea102010.o Belinea103040.o \
	Belinea104010.o Belinea105035.o Belinea105046.o \
	Belinea105080.o Belinea106020.o Belinea107015.o \
	Belinea107020.o Belinea107035.o Belinea107050.o \
	Belinea107060.o Belinea107065.o Belinea108015.o \
	Belinea108050.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_MONITOR_VENDOR),MAXDATA)
default.entry: $(CONFIG_MONITOR_MODEL).o
endif
