default.exit: modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o

SUBSYSTEMS = chipset/chipset.o clock/clock.o ramdac/ramdac.o monitor/monitor.o board/board.o
#	system/system.o

.PHONY: modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o
modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o:
	@echo writing driver to $@
	mkdir -p modules/$(CONFIG_BOARD_VENDOR)
	$(LD_BIN) -o $@ -r $(SUBSYSTEMS)

distclean.exit:
	rm -rf modules
