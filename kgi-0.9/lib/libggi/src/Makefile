CC_OPT_DEBUG=-DDEBUG_LEVEL=1

SHARED_OBJECTS = libGGI.so

default.entry: $(SHARED_OBJECTS)

major = $(libGGI_MAJOR)
minor = $(libGGI_MINOR)
patch = $(libGGI_PATCH)

install.exit: $(SHARED_OBJECTS)
	install -d $(DIR_INSTALL)/lib
	install -m 555 libGGI.so $(DIR_INSTALL)/lib/libGGI.so.$(major).$(minor).$(patch)
	cd $(DIR_INSTALL)/lib; \
	ln -f -s libGGI.so.$(major) libGGI.so; \
	ln -f -s libGGI.so.$(major).$(minor).$(patch) libGGI.so.$(major)

CC_LIBS = -ldl
libGGI.so: libGGI.o dll.o
