
default.exit:: ../dix.o

OBJECTS = atom.o	\
	colormap.o	\
	cursor.o	\
	devices.o	\
	dispatch.o	\
	dixutils.o	\
	events.o	\
	extension.o	\
	gc.o		\
	globals.o	\
	glyphcurs.o	\
	grabs.o		\
	main.o		\
	property.o	\
	resource.o	\
	swaprep.o	\
	swapreq.o	\
	tables.o	\
	window.o	\
	initatoms.o	\
	dixfonts.o	\
	privates.o	\
	pixmap.o	\
	xpstubs.o

#if !HasFfs
#	OBJECTS_FFS = ffs.o

../dix.o: $(OBJECTS) $(OBJECTS_FFS)
	$(LD) -r -o $@ $^

#
#	how to generate initatoms.c and <X11/Xatom.h> from BuiltInAtoms
#
initatoms.c ../include/X11/Xatom.h: BuiltInAtoms buildatoms
	cd $(DIR_SOURCE); ./buildatoms
