#
#	C/C++ compiler Configuration
#

#	helper functions to set default parameters for the various compilers
#
function cc_gnuc {
	#
	#	GNU C/C++ compiler settings
	#
	CC_BIN=${CC_BIN:-"gcc"}
	CC_LIBS=${CC_LIBS}
	CC_OPT=${CC_OPT:-""}
#	former value of CC_OPT: -ansi -traditional-cpp"}

	CC_OPT_WARN=${CC_OPT_WARN:-"-W -Wall"}
	CC_OPT_DEBUG=${CC_OPT_DEBUG:-"-g"}
	CC_OPT_OPTIMIZE=${CC_OPT_OPTIMIZE:-"-O2"}
	CC_OPT_PIC=${CC_OPT_PIC:-"-fPIC"}

	CXX_BIN=${CXX_BIN:-"g++"}
	CXX_LIBS=${CXX_LIBS}
	CXX_OPT=${CXX_OPT}

	CC_RULES=cc++

	if [ "$HOST_CPU" != "$HOST_BUILD_CPU" ]
	then
		CC_OPT_TARGET="-b $HOST_CPU"
	fi
}

function cc_sgic {

	#	SGI (IRIX) compiler C/C++ compiler settings
	#
	CC_BIN=${CC_BIN:-"cc"}
	CC_LIBS=${CC_LIBS}
	CC_OPT=${CC_OPT:-""}
#	former value of CC_OPT: -ansi -traditional-cpp

	CC_OPT_WARN=${CC_OPT_WARN:-"-fullwarn"}
#	additional value possible: -pedantic"
	CC_OPT_DEBUG=${CC_OPT_DEBUG:-"-g3"}
	CC_OPT_OPTIMIZE=${CC_OPT_OPTIMIZE:-"-O2"}
	CC_OPT_PIC=${CC_OPT_PIC:-"-KPIC"}

	CXX_BIN=${CXX_BIN:-"CC"}
	CXX_LIBS=$CXX_LIBS
	CXX_OPT=${CXX_OPT}

	CC_RULES=cc++

	if [ "$HOST_CPU" != "$HOST_BUILD_CPU" ]
	then
		echo "cc/configure.sh: cross-compile not (yet) supported for SGI C compilers"
		exit 1
	fi
}

#
#	the actual compiler configuration module
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--cc-compiler={gnuc|egcs|sgic}]"
	echo -n " [--cc-bin=<cc>] [--cc-libs=<libs>]"
	echo -n " [--cc-library-path=<path>] [--cc-include-path=<path>]"
	echo -n " [--cxx-bin=<c++>] [--cxx-libs=<libs>]"
	echo -n " [--cxx-library-path=<path>] [--cxx-include-path=<path>]"
	echo -n " [--cc-opt-warn=<warn>] [--cc-opt-debug=<debug>]"
	echo -n " [--cc-opt-optimize=<optimize>] [--cc-opt-target=<target>]"
	echo -n " [--cc-opt-pic=<pic>]"
	;;

help-options)
	cat <<end-of-options
	--cc-compiler={gnuc|egcs|sgic}
		optional, default: build host OS dependent
		C/C++ compiler to use. Currently recognized compilers are:
		gnuc	GNU-C/C++ compiler
		egcs	Experimental GNU compiler system
		sgic	SGI C/C++ compiler that ships with IRIX

	--cc-bin=<cc>
		optional, default: compiler dependent
		Set C compiler binary and general options to use when
		compiling C/C++ code.

	--cc-libs=<libs>
		optional, default: "" (empty string)
		Set libraries to link with when building C programs or
		libraries.

	--cc-library-path=<path>
		optional, default: "" (empty string)
		Set path to search libraries in. This is a space
		separated list of directories (absolute path names!) to	
		search for libraries when linking.

	--cc-include-path=<path>
		optional, default: ". DIR_TOP_BUILD/include" (empty string)
		Set path to search headers in. This is a space
		separated list of directories (absolute path names!) to
		search for includes when compiling C code.

	--cc-opt-warn=<warn>
		optional, default: compiler dependent
		Set warning options to use when compiling C/C++ code.

	--cc-opt-debug=<debug>
		optional, default: compiler dependent
		Set debugging options (e.g. options to include symbol
		information for debuggers) to use when compiling C/C++ code.

	--cc-opt-optimize=<opt>
		optional, default: compiler dependent
		Set optimization options to use when compiling C/C++ code.

	--cc-opt-target=<target>
		optional, default: compiler dependent
		Set compiler options to produce code for target OS/CPU when
		when compiling C/C++ code.

	--cc-opt-pic=<pic>
		optional, default: compiler dependent
		Set compiler option to produce position independent code
		(e.g. for shared libraries).

	--cxx-bin=<c++>
		optional, default: compiler dependent
		Set C++ compiler binary to use.

	--cxx-libs=<libs>
		optional, default: "" (empty string)
		Set libraries to link with when building C++ programs or
		libraries.

	--cxx-library-path=<path>
		optional, default: CC_LIBRARY_PATH
		Set path to search libraries in. This is a space
		separated list of directories (absolute path names!) to	
		search for libraries when linking.

	--cxx-include-path=<path>
		optional, default: CC_LIBRARY_PATH
		Set path to search headers in. This is a space
		separated list of directories (absolute path names!) to
		search for includes when compiling C code.

end-of-options
	;;

config-parse)
	case $HOST_BUILD_OS in
	Linux)	CC_COMPILER=gnuc ;;
	Hurd)	CC_COMPILER=gnuc ;;
	IRIX)	CC_COMPILER=sgic ;;
	*)	echo "cc/configure.sh: Error: no default C/C++ compiler for $HOST_BUILD_OS"
		exit 1
	esac

	for ARG; do case $ARG in

	--cc-compiler=*)	CC_COMPILER=`configure_arg $ARG` ;;

	--cc-bin=*)		CC_BIN=`configure_arg $ARG` ;;
	--cc-libs=*)		CC_LIBS=`configure_arg $ARG` ;;
	--cc-library-path=*)	CC_LIBRARY_PATH=`configure_arg $ARG` ;;
	--cc-include-path=*)	CC_INCLUDE_PATH=`configure_arg $ARG` ;;

	--cc-opt-warn=*)	CC_OPT_WARN=`configure_arg $ARG` ;;
	--cc-opt-debug=*)	CC_OPT_DEBUG=`configure_arg $ARG` ;;
	--cc-opt-optimize=*)	CC_OPT_OPTIMIZE=`configure_arg $ARG` ;;
	--cc-opt-target=*)	CC_OPT_TARGET=`configure_arg $ARG` ;;
	--cc-opt-pic=*)		CC_OPT_PIC=`configure_arg $ARG` ;;

	--cxx-bin=*)		CXX_BIN=`configure_arg $ARG` ;;
	--cxx-libs=*)		CXX_LIBS=`configure_arg $ARG` ;;
	--cxx-library-path=*)	CXX_LIBRARY_PATH=`configure_arg $ARG` ;;
	--cxx-include-path=*)	CXX_INLCUDE_PATH=`configure_arg $ARG` ;;

	--cc-*|--cxx-*)
		echo "cc/configure.sh: unknown module option '$ARG'"
		exit 1
	esac; done

	case $CC_COMPILER in 
	gnuc|egcs)	cc_gnuc ;;
	sgic)		cc_sgic ;;
	*)	echo "cc/configure.sh: unknown compiler system $CC_COMPILER"
		exit 1
	esac
	;;

config-help)
	cat <<end-of-config-help
C compiler options (cc/configure.sh):
	CC_OPT				Compiler options to compile C
	CC_BIN				C compiler binary to use
	CC_LIBS				additional libraries for C programs
	CC_INCLUDE_PATH			search path for C includes
	CC_LIBRARY_PATH			search path for C libraries

C/C++ compiler (cc/configure.sh):
	CC_COMPILER			compiler system to use
	CC_OPT_WARN			warning options
	CC_OPT_DEBUG			debugging options
	CC_OPT_OPTIMIZE			optimization options
	CC_OPT_TARGET			target selection options
	CC_OPT_PIC			position independent code options

C++ compiler (cc/configure.sh):
	CXX_OPT				compiler options to compile C++
	CXX_BIN				C++ compiler binary to use
	CXX_LIBS			additional libraries for C++ programs
	CXX_INCLUDE_PATH		search path for C++ includes
	CXX_LIBRARY_PATH		search path for C++ libraries

end-of-config-help
	;;

config-file)
	mkdir -p $DIR_TOP_BUILD/include
	mkdir -p $DIR_TOP_BUILD/lib

	cat <<end-of-config-file
#	C/C++ compiler package
#
CC_BIN="$CC_BIN"
CC_LIBS="$CC_LIBS"
CC_OPT="$CC_OPT"
CC_INCLUDE_PATH="${CC_INCLUDE_PATH:-$DIR_TOP_BUILD/include}"
CC_LIBRARY_PATH="${CC_LIBRARY_PATH:-$DIR_TOP_BUILD/lib}"

CC_OPT_WARN="$CC_OPT_WARN"
CC_OPT_DEBUG="$CC_OPT_DEBUG"
CC_OPT_OPTIMIZE="$CC_OPT_OPTIMIZE"
CC_OPT_TARGET="$CC_OPT_TARGET"
CC_OPT_PIC="$CC_OPT_PIC"

CXX_BIN="$CXX_BIN"
CXX_LIBS="$CXX_LIBS"
CXX_OPT="$CXX_OPT"
CXX_INCLUDE_PATH="${CXX_INCLUDE_PATH:-${CC_INCLUDE_PATH:-$DIR_TOP_BUILD/include}}"
CXX_LIBRARY_PATH="${CXX_LIBRARY_PATH:-${CC_LIBRARY_PATH:-$DIR_TOP_BUILD/lib}}"
end-of-config-file
	;;

config-makefile)
	cat $DIR_TOOLS/??cc/rules
	cat $DIR_TOOLS/??cc/rules.$CC_RULES
	;;

*)
	echo "cc/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
