
default.exit: ../xaa.so

#
#	stems for special objects
#
STEMS = xaaBitmap	\
	xaaTEGlyph	\
	xaaBitmap3	\
	xaaStipple
#	xaaNonTEGlyph

OBJECTS_LSB_FIRST = $(patsubst %,%-lsb-first.o,$(STEMS))
OBJECTS_LSB_FIXED = $(patsubst %,%-lsb-fixed.o,$(STEMS))
OBJECTS_MSB_FIRST = $(patsubst %,%-msb-first.o,$(STEMS))
OBJECTS_MSB_FIXED = $(patsubst %,%-msb-fixed.o,$(STEMS))

OBJECTS = xaaInit.o	\
	xaaGC.o		\
	xaaInitAccel.o	\
	xaaFallback.o	\
	xaaBitBlt.o	\
	xaaCpyArea.o	\
	xaaGCmisc.o	\
	xaaCpyWin.o	\
	xaaCpyPlane.o	\
	xaaFillRect.o	\
	xaaTEText.o	\
	xaaNonTEText.o	\
	xaaPCache.o	\
	xaaSpans.o	\
	xaaROP.o	\
	xaaImage.o	\
	xaaPaintWin.o	\
	xaaRect.o	\
	xaaLineMisc.o	\
	xaaBitOrder.o	\
	xaaFillPoly.o	\
	xaaWideLine.o	\
	xaaTables.o	\
	xaaFillArc.o	\
	xaaLine.o	\
	xaaSeg.o	\
	xaaDashLine.o	\
	xaaDashSeg.o	\
	xaaOverlay.o	\
	xaaOffscreen.o	\
	xaaOverlayDF.o	\
	xf86.o		\
	xf86fbman.o

../xaa.o: $(OBJECTS)		\
	$(OBJECTS_LSB_FIRST)	\
	$(OBJECTS_LSB_FIXED)	\
	$(OBJECTS_MSB_FIRST)	\
	$(OBJECTS_MSB_FIXED)
	$(LD) -r -o $@ $^

../xaa.so: ../xaa.o

#
#	implicit rules to build special objects
#
%-lsb-first.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DLSBFIRST -c $< -o $@

%-msb-first.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DMSBFIRST -c $< -o $@

%-lsb-fixed.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DLSBFIRST -DFIXEDBASE -c $< -o $@

%-msb-fixed.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DMSBFIRST -DFIXEDBASE -c $< -o $@

