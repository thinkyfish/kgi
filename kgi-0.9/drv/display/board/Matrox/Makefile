OBJECTS = 1x64sg.o G200_AGP.o G200_AGP_OEM.o G200_PCI.o G200_PCI_OEM.o G400.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),Matrox)
default.entry: $(CONFIG_BOARD_MODEL).o
endif
