ifeq ($(HOST_OS),Linux)

CC_OPT_WARN=-Wall -fomit-frame-pointer

OBJECTS = kgim-0.9.o

all.entry:: $(OBJECTS)

ifeq ($(CONFIG_SYSTEM_VENDOR),Linux)
default.entry:: $(CONFIG_SYSTEM_MODEL).o
endif

endif
