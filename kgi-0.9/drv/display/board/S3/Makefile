OBJECTS = ViRGE.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_BOARD_VENDOR),S3)
default.entry:: $(CONFIG_BOARD_MODEL).o
endif
