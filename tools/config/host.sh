#
#	find out details about the host environment
#
case $CONFIG_ACTION in
help-syntax)
	echo -n " [--host-cpu=<cpu>] [--host-os=<os>] [--host-os-release=<rel>]"
	;;

help-options)
	cat <<end-of-options
	--host-name=<name>
		optional, default: build host name
		Set the full qualified target host name. 

	--host-cpu=<cpu>
		optional, default: build host CPU type
		Set the target host CPU type. <cpu> must be the output of 
		"uname -m" on the target host.

	--host-os=<os>
		optional, default: build host OS type
		Set the target host OS. <os> must be the output of "uname -s"
		on the target host.

	--host-os-release=<rel>
		optional, default: build host OS release
		Set the target host OS release. <rel> must be the output of
		"uname -r" on the target host.

end-of-options
	;;

config-parse)
	HOST_BUILD_NAME=`uname -n`
	HOST_BUILD_CPU=`uname -m`
	HOST_BUILD_OS=`uname -s`
	HOST_BUILD_OS_RELEASE=`uname -r`

	for ARG; do case $ARG in
	--host-name=*)
		HOST_NAME=`configure_arg $ARG`
		;;

	--host-cpu=*)
		HOST_CPU=`configure_arg $ARG`
		;;

	--host-os=*)
		HOST_OS=`configure_arg $ARG`
		;;

	--host-os-release=*)
		HOST_OS_RELEASE=`configure_arg $ARG`
		;;

	--host-*)
		echo "host.sh: unknown host configuration option"
	esac; done
	;;

config-help)
	cat <<end-of-config-help
build host environment (config/host.sh):
	HOST_BUILD_NAME			full qualified build host name
	HOST_BUILD_CPU			build host CPU architecture
	HOST_BUILD_OS			build host OS type
	HOST_BUILD_OS_RELEASE		build host OS release

target host environment (config/host.sh):
	HOST_NAME			full qualified target host name
	HOST_CPU			target host CPU architecture
	HOST_OS				target host OS type
	HOST_OS_RELEASE			target host OS release

end-of-config-help
	;;

config-file)
	cat <<end-of-config
#
#	build and target host info
#
HOST_BUILD_NAME=$HOST_BUILD_NAME
HOST_BUILD_CPU=$HOST_BUILD_CPU
HOST_BUILD_OS=$HOST_BUILD_OS
HOST_BUILD_OS_RELEASE=$HOST_BUILD_OS_RELEASE
HOST_NAME=${HOST_NAME:-$HOST_BUILD_NAME}
HOST_CPU=${HOST_CPU:-$HOST_BUILD_CPU}
HOST_OS=${HOST_OS:-$HOST_BUILD_OS}
HOST_OS_RELEASE=${HOST_OS_RELEASE:-$HOST_BUILD_OS_RELEASE}
end-of-config
	;;

config-makefile)
	;;

*)
	echo "config/host.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
