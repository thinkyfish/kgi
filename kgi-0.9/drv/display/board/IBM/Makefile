OBJECTS = VGA.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),IBM)
default.entry:: $(CONFIG_BOARD_MODEL).o
endif
