
default.entry:
	@echo "clock: $(CONFIG_CLOCK_VENDOR) $(CONFIG_CLOCK_MODEL) " \
		"($(CONFIG_CLOCK))"

default.exit: $(CONFIG_CLOCK_VENDOR)/$(CONFIG_CLOCK_MODEL).o
	cp $< clock.o

distclean.exit:
	rm -f .select-clock .clock

