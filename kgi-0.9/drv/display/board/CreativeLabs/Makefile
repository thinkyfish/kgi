OBJECTS = GraphicsBlaster_RivaTNT2_Ultra.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),CreativeLabs)
default.entry: $(CONFIG_BOARD_MODEL).o
endif
