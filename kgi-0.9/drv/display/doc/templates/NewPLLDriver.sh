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
	echo "creating clock/$VENDOR/$2"
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
	}" < $DIR_TEMPLATES/$1 > clock/$VENDOR/$2
}

if [ ! -d clock/$VENDOR ]
then
	echo "creating vendor subdirectory clock/$VENDOR"
	mkdir -p clock/$VENDOR

	echo "creating vendor configure script clock/$VENDOR/.configure"
	touch clock/$VENDOR/.configure

	echo "creating vendor name file chipset/$VENDOR/.configure"
	echo $VENDOR_NAME > clock/$VENDOR/.vendor
fi

if [ ! -f clock/$VENDOR/Makefile ]
then
	echo "creating vendor Makefile clock/$VENDOR/Makefile"
	CopyTemplate pll-Makefile Makefile
else
	echo "vendor Makefile clock/$VENDOR/Makefile exists."
	echo "Please add the new driver by hand to clock/$VENDOR/Makefile"
fi

CopyTemplate pll-meta.h $META_CPP-meta.h
CopyTemplate pll-meta.c $META_CPP-meta.c
CopyTemplate pll-bind.h $META_CPP-bind.h
CopyTemplate pll-bind.c $META_CPP-bind.c
