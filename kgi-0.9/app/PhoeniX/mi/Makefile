
default.exit:: ../mi.o

OBJECTS = mivaltree.o	\
	mipolyseg.o	\
	mipolyrect.o	\
	mipoly.o	\
	mipolycon.o	\
	mipolygen.o	\
	mipolyutil.o	\
	mifillrct.o	\
	miwideline.o	\
	mispans.o	\
	miarc.o		\
	mizerarc.o	\
	mifillarc.o	\
	miwindow.o	\
	micursor.o	\
	miregion.o	\
	mipolytext.o	\
	mibitblt.o	\
	mipolypnt.o	\
	mipushpxl.o	\
	miexpose.o	\
	miglblt.o	\
	mizerline.o	\
	mizerclip.o	\
	mifpolycon.o	\
	midash.o	\
	mibstore.o	\
	mibank.o	\
	miinitext.o	\
	mieq.o		\
	mipointer.o	\
	misprite.o	\
	midispcur.o	\
	miscrinit.o	\
	miclipn.o	\
	migc.o		\
	micmap.o	\
	mioverlay.o

#	if !HasCbrt
#	OBJECTS_CBRT = cbrt.c

../mi.o: $(OBJECTS) $(OBJECTS_CBRT)
	$(LD) -r -o $@ $^
