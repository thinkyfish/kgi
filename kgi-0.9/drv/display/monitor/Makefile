default.entry:
	@echo "monitor: $(CONFIG_MONITOR_VENDOR) $(CONFIG_MONITOR_MODEL)"

default.exit: $(CONFIG_MONITOR_VENDOR)/$(CONFIG_MONITOR_MODEL).o
	cp $< monitor.o

distclean.exit: clean.exit
	rm -f .monitor-select .monitor

