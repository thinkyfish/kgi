#
#	directory configuration
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--dir-build=<path>] [--dir-install=<path>]"
	echo -n " [--dir-prefix=<path>]"
	;;

help-options)
	cat <<end-of-options
	--dir-build=<path>
		optional, default: current working directory
		Set path to top-level build directory.

	--dir-install=<path>
		optional, default: "" (empty string)
		Set path to top-level installation directory. On installation
		files will be copied into a tree below this directory.

	--dir-prefix=<path>
		optional, default: user's home directory
		Set prefix to append to the installation directory. This is 
		the location where the installed files are expected when 
		executing on the target machine.
		
end-of-options
	;;

config-parse)
	#	DIR_TOP_BUILD default is set by configure
	#	DIR_TOP_SOURCE is set by configure
	DIR_INSTALL=${DIR_INSTALL:-}
	DIR_PREFIX=${DIR_PREFIX:-$HOME}
	for ARG; do case $ARG in

	--dir-build=*)
		DIR_TOP_BUILD=`configure_arg $ARG`
		;;

	--dir-install=*)
		DIR_INSTALL=`configure_arg $ARG`
		;;

	--dir-prefix=*)
		DIR_PREFIX=`configure_arg $ARG`
		;;

	--dir-*)
		echo "dirs.sh: unknown directory option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
directories (config/dirs.sh):
	DIR_TOP_BUILD			top-level build directory
	DIR_TOP_SOURCE			top-level source directory
	DIR_INSTALL			top-level installation directory
	DIR_PREFIX			installation prefix directory

end-of-config-help
	;;

config-file)
	cat <<end-of-config
#
#	directories
#
DIR_TOP_BUILD=$DIR_TOP_BUILD
DIR_TOP_SOURCE=$DIR_TOP_SOURCE
DIR_INSTALL=$DIR_INSTALL
DIR_PREFIX=$DIR_PREFIX
DIR_TOOLS=$DIR_TOOLS
end-of-config
	;;

config-makefile)
	cat $DIR_TOOLS/config/dirs.rules
	;;

*)
	echo "config/dirs.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
