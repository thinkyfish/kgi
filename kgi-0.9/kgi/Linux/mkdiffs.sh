#!/bin/sh
#
#	Copyright (C)	2000	Steffen Seeger
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#

. ./.config

REJECTS=`find linux -name *.rej -print`
if [ "xxx$REJECTS" = "xxx" ]
then
	if [ -f linux/drivers/char/Makefile.orig ]
	then
		echo -n "patches/$LINUX_VERSION-char-Makefile.diff ... "
		diff -u linux/drivers/char/Makefile.orig 		\
			linux/drivers/char/Makefile 			\
			> patches/$LINUX_VERSION-char-Makefile.diff
		rm linux/drivers/char/Makefile.orig
		echo done.
	fi

	echo -n "patches/$LINUX_VERSION.diff ... "
	for i in `find linux/ -name *.orig | sed "s/.orig$//"`
	do
		chmod a+r $i.orig $i
		diff -u $i.orig $i >> patches/$LINUX_VERSION.diff
	done
	echo done.
else
	echo "rejected patches found. Please resolve conflicts."
	echo "files found: $REJECTS"
	exit 1
fi
