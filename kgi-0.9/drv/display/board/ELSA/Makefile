OBJECTS = WinnerOffice2000.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),ELSA)
default.entry: $(CONFIG_BOARD_MODEL).o
endif
