#!/bin/sh
# -----------------------------------------------------------------------------
#	main configuration package
# -----------------------------------------------------------------------------
#
#	Copyright (C)	2000	Steffen Seeger
#

DIR_TOOLS=$DIR_TOP_SOURCE/tools

function configure_arg() {

	echo $* | sed "s/[^=]*=//"
}

#
#	First configure all modules in $DIR_TOOLS
#
MODULES=`find $DIR_TOOLS/. -name configure.sh -print | sed "s%/configure.sh$%%" | sort -n`

function configure_modules() {

	source $DIR_TOOLS/config/dirs.sh
	source $DIR_TOOLS/config/host.sh

	for module in $MODULES
	do
		source $module/configure.sh
	done
}

case $1 in
--help)
	case "/$2/" in
	"/config-file/")
		CONFIG_ACTION=config-help
		configure_modules "$@"
		exit 0
		;;
	"//")
		;;
	*)
		if test -f `echo $DIR_TOOLS/??$2/configure.sh`
		then
			echo -n "usage: configure [--help [config-file|<tool>]]"
			CONFIG_ACTION=help-syntax
			source $DIR_TOOLS/??$2/configure.sh

			echo; echo "Options:"
			CONFIG_ACTION=help-options
			source $DIR_TOOLS/??$2/configure.sh

			echo ".config file variables"
			echo "----------------------"
			echo
			CONFIG_ACTION=config-help
			source $DIR_TOOLS/??$2/configure.sh
			exit 0
		else
			echo "config/main.sh: no such tool '$2'"
			exit 1
		fi
	esac

	echo -n "usage: configure [--help [config-file|<tool>]]"
	CONFIG_ACTION=help-syntax
	configure_modules "$@"
	echo

	cat <<end-of-options
Options:
	--help [config-file|<tool>]
		optional
		Print this help text. If the "config-file" option is given,
		explain the semantics of the variables set in .config.
		Any other argument will be interpreted as a tool name and
		config options for that tool only will be printed.

end-of-options

	CONFIG_ACTION=help-options
	configure_modules "$@"
	exit 0
	;;
esac

CONFIG_ACTION=config-parse
configure_modules "$@"

FILE_CONFIG=$DIR_TOP_BUILD/.config
CONFIG_ACTION=config-file
configure_modules "$@" > $FILE_CONFIG
source $FILE_CONFIG

FILE_GNUMAKEFILE=$DIR_TOP_BUILD/GNUmakefile
CONFIG_ACTION=config-makefile
configure_modules "$@" > $FILE_GNUMAKEFILE
cat >> $FILE_GNUMAKEFILE <<-end-of-trailer

#       user defined dependencies
#
-include \$(DIR_SOURCE)/Makefile
-include \$(DIR_BUILD)/Makefile 

end-of-trailer

#
#	Now that all tools are configured we have the top-level
#	.config and GNUmakefile. Thus we can proceed to configure
#	all subdirectories starting from $DIR_TOP_SOURCE.
#

function do_reconfig () {

	if test -f $DIR_BUILD/.config.tmp
	then
		rm $DIR_BUILD/.config
		mv $DIR_BUILD/.config.tmp $DIR_BUILD/.config
		FILE_CONFIG=$DIR_BUILD/.config
	fi
	if test -f $DIR_BUILD/GNUmakefile.tmp
	then
		rm $DIR_BUILD/GNUmakefile
		mv $DIR_BUILD/GNUmakefile.tmp $DIR_BUILD/GNUmakefile
		FILE_GNUMAKEFILE=$DIR_BUILD/GNUmakefile
	fi
}

function do_recursion () {

	cd $DIR_SOURCE
	SUBDIRS=`echo */.configure | sed "s%/.configure%%g"`

	cd $DIR_BUILD

	if test -x $DIR_SOURCE/.configure
	then
		$DIR_SOURCE/.configure "$@"
		do_reconfig
	else
		source $DIR_SOURCE/.configure
		do_reconfig
	fi

	if test -x $DIR_SOURCE/.configure.c
	then
		cc $DIR_SOURCE/.configure.c -o $DIR_BUILD/.configure
		$DIR_BUILD/.configure "$@"
		do_reconfig
	fi

	if [ "$SUBDIRS" != "*" ]
	then
		for DIR in $SUBDIRS
		do (
			echo "$1- $DIR"

			DIR_BUILD=$DIR_BUILD/$DIR
			DIR_SOURCE=$DIR_SOURCE/$DIR

			if ! test -d $DIR_BUILD
			then
				mkdir $DIR_BUILD
			fi

			ln -f -s $FILE_GNUMAKEFILE $DIR_BUILD/GNUmakefile
			ln -f -s $FILE_CONFIG $DIR_BUILD/.config

			do_recursion "$1   |"
		); done
	fi
}

DIR_BUILD=$DIR_TOP_BUILD
DIR_SOURCE=$DIR_TOP_SOURCE

do_recursion " |" "$@"

echo "done."
