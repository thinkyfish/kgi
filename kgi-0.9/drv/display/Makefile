default.exit:: modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o

SUBSYSTEMS = chipset/chipset.o clock/clock.o ramdac/ramdac.o monitor/monitor.o board/board.o
#	system/system.o

.PHONY: modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o
modules/$(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o:
	@echo writing driver to $@
	mkdir -p modules/$(CONFIG_BOARD_VENDOR)
	$(LD) -o $@ -r $(SUBSYSTEMS)

clean.exit::
	rm -f menu welcome.*.htf

distclean.exit::
	rm -rf modules

welcome.en.htf: $(DIR_SOURCE)/status2htf.awk $(wildcard $DIR_SOURCE/*/*/status)
	( cd $(DIR_SOURCE);						\
	echo "./vendors";						\
	cat ./vendors;							\
	for dir in board/ chipset/ clock/ ramdac/ monitor/; do		\
		for file in `find ./$$dir -name status -print`; do \
			echo $$file;					\
			cat $$file;					\
		done;							\
	done ) | $(AWK_BIN) -v revision=`date +%Y-%m-%d/%H:%M` -F'\t' -f $(DIR_SOURCE)/status2htf.awk > $@

menu:
	(	echo -e "rel\ten\t\"Boards\"\twelcome.en.html#board";	\
		echo -e "rel\ten\t\"Chipsets\"\twelcome.en.html#chipset"; \
		echo -e "rel\ten\t\"Clock Chips\"\t\twelcome.en.html#clock";\
		echo -e "rel\ten\t\"DACs\"\twelcome.en.html#ramdac";	\
		echo -e "rel\ten\t\"Monitors\"\twelcome.en.html#monitor"; \
	) > $@
