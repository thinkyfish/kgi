CC_OPT_DEBUG=-DDEBUG_LEVEL=1


SHARED_OBJECTS = api.so KGI.so

default.exit:	$(SHARED_OBJECTS)

install.exit: $(SHARED_OBJECTS)
	install -d $(DIR_INSTALL)/lib/GGI
	for i in $(SHARED_OBJECTS); \
		do install -m 555 $$i $(DIR_INSTALL)/lib/GGI; \
	done
