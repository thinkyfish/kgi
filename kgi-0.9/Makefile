.PHONY: config
 
config:
	./configure

snapshot:
	cd ..; tar czf kgi-0.9-`date +%Y%m%d`.tar.gz kgi-0.9/

distclean.exit::
	rm -rf $(DIR_TOP_BUILD)/include

ifeq (.config, $(wildcard .config))
#
#	system header installation
#
include .config
endif
