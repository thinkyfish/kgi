default.entry:
	@echo "board: $(CONFIG_BOARD_VENDOR) $(CONFIG_BOARD_MODEL)"

default.exit: $(CONFIG_BOARD_VENDOR)/$(CONFIG_BOARD_MODEL).o
	cp $< board.o

distclean.exit: clean.exit
	rm -f .board-select .board

