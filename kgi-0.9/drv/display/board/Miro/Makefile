OBJECTS = miroCrystal20SV.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),Miro)
default.entry: $(CONFIG_BOARD_MODEL).o
endif
