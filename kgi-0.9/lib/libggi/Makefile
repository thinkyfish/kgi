.PHONY: config

config:
	ln -f -s config/common/GNUmake.rules GNUmakefile
	config/configure

install.exit:
	install -d $(DIR_INSTALL)/lib
	install -d $(DIR_INSTALL)/include/ggi
	for i in include/ggi/*.h; \
		do install -m 444 $$i $(DIR_INSTALL)/$$i; \
	done
