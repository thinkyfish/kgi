#!/bin/sh

. ./.config

X11_XC_PATCH=patches/XFree86-$X11_MAJOR.$X11_MINOR.$X11_PATCH.diff

REJECTS=`find xc/ -name *.rej -print`
if [ "xxx$REJECTS" = "xxx" ]
then
	echo -n "$X11_XC_PATCH ... "
	(for i in `find xc/ -name *.orig | sed "s/.orig$//"`
	do
		chmod a+r $i.orig $i
		diff -u $i.orig $i
	done) > $X11_XC_PATCH
	echo done.
else
	echo "rejected patches found. Please resolve conflicts."
	echo "files found: $REJECTS"
	exit 1
fi
