default.entry:
	@echo "monitor: $(CONFIG_MONITOR_VENDOR) $(CONFIG_MONITOR_MODEL)"

default.exit: monitor.o

distclean.exit: clean.exit
	rm -f .monitor-select .monitor


monitor.o: $(CONFIG_MONITOR_VENDOR)/$(CONFIG_MONITOR_MODEL).o	\
	$(CONFIG_MONITOR_DRIVER)/$(CONFIG_MONITOR_TIMING).o
	$(LD_BIN) -o monitor.o -r $^
