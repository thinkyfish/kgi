#!/bin/sh
#
#	Copyright (C)	2000	Steffen Seeger
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#

DIR_TEMPLATES=`echo $0 | sed "s%/[^/]*$%%"`

if [ -f "$1" ]
then
	echo Reading $1
	. $1 
else
	echo "Usage: $0 <driver spec>"
	echo "where driver spec looks like"
	cat <<-end-of-sample-spec

VENDOR_NAME="Xyz Incorporated"		# vendor string
VENDOR="Xyz"				# vendor ID string
MODEL_NAME="0815"			# model string
AUTHOR_NAME="Joe Author"		# full author name
AUTHOR="Joe_Author"			# author user ID
META_CPP="XYZ08xx"			# C pre-processor meta language prefix
META_C="xyz08xx"			# C meta language prefix
end-of-sample-spec
	exit 1
fi

function CopyTemplate()
{
	echo "creating ramdac/$VENDOR/$2"
	awk "{	
		gsub(/##VENDOR##/, 	\"$VENDOR\");
		gsub(/##VENDOR_NAME##/, \"$VENDOR_NAME\");
		gsub(/##MODEL##/, 	\"$META_C\");
		gsub(/##MODEL_NAME##/,	\"$MODEL_NAME\");
		gsub(/##AUTHOR_NAME##/,	\"$AUTHOR_NAME\");
		gsub(/##AUTHOR##/,	\"$AUTHOR\");
		gsub(/##META##/,	\"$META_CPP\");
		gsub(/##meta##/,	\"$META_C\");
		print;
	}" < $DIR_TEMPLATES/$1 > ramdac/$VENDOR/$2
}

if [ ! -d ramdac/$VENDOR ]
then
	echo "creating vendor subdirectory ramdac/$VENDOR"
	mkdir -p ramdac/$VENDOR

	echo "creating vendor configure script ramdac/$VENDOR/.configure"
	touch ramdac/$VENDOR/.configure
fi

echo "adding driver to status file ramdac/$VENDOR/status"
echo "$META_CPP	$MODEL_NAME	10	$AUTHOR" >> ramdac/$VENDOR/status

if [ ! -f ramdac/$VENDOR/Makefile ]
then
	echo "creating vendor Makefile ramdac/$VENDOR/Makefile"
	CopyTemplate ramdac-Makefile Makefile
else
	echo "vendor Makefile ramdac/$VENDOR/Makefile exists."
	echo "Please add this driver by hand."
fi

CopyTemplate ramdac.h $META_CPP.h
CopyTemplate ramdac-meta.h $META_CPP-meta.h
CopyTemplate ramdac-meta.c $META_CPP-meta.c
CopyTemplate ramdac-bind.h $META_CPP-bind.h
CopyTemplate ramdac-bind.c $META_C-bind.c
