OBJECTS = Permedia2v.o

all.entry: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),3Dlabs)
default.entry: $(CONFIG_BOARD_MODEL).o
endif
