#
#	Linux specific configuration
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--linux-build-distribution=<dist>]"
	echo -n " [--linux-distribution=<dist>]"
	echo -n " [--linux-release=<rel>] [--linux-build-release=<rel>]"
	;;

help-options)
	cat <<end-of-options
	--linux-build-distribution=<dist>
		optional, default: auto-detected
	--linux-distribution=<dist>
		optional, default: build-distribution
		Set the base distribution type for build and target host.
		If you specify a installation directory, give this option 
		if no system is installed there.
		<dist> may be one of "RedHat" or "unknown".

	--linux-build-release=<rel>
		optional, default: auto-detected
	--linux-release=<rel>
		optional, default: build-release
		Set the distribution release for build and target host.
		If you specify a installation directory, give this option
		if no system is installed there.

end-of-options
	;;

config-parse)
	#	test for RedHat distribution(s)
	#
	if test -f /etc/redhat-release
	then
		LINUX_BUILD_DISTRIBUTION=RedHat
		LINUX_BUILD_RELEASE=`sed "s/[^r]*release //;s/[^0-9]*$//" /etc/redhat-release`
	fi
	if test -f $DIR_INSTALL/etc/redhat-release
	then
		LINUX_DISTRIBUTION=RedHat
		LINUX_RELEASE=`sed "s/[^r]*release //;s/[^0-9]*$//" $DIR_INSTALL/etc/redhat-release`
	fi

	for ARG; do case $ARG in

	--linux-build-distribution=*)
		LINUX_BUILD_DISTRIBUTION=`configure_arg $ARG`
		;;

	--linux-build-release=*)
		LINUX_BUILD_RELEASE=`configure_arg $ARG`
		;;

	--linux-distribution=*)
		LINUX_DISTRIBUTION=`configure_arg $ARG`
		;;

	--linux-release=*)
		LINUX_RELEASE=`configure_arg $ARG`
		;;

	--linux-*)
		echo "Linux.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
Linux variables (config/Linux.sh):
	LINUX_BUILD_DISTRIBUTION		build distribution
	LINUX_BUILD_RELEASE			build distribution release
	LINUX_DISTRIBUTION			(install-)target distribution
	LINUX_RELEASE				target distribution release

end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
#
#	Linux options
#
LINUX_BUILD_DISTRIBUTION=${LINUX_BUILD_DISTRIBUTION:-unknown}
LINUX_BUILD_RELEASE=${LINUX_BUILD_RELEASE:-unknown}
LINUX_DISTRIBUTION=${LINUX_DISTRIBUTION:-${LINUX_BUILD_DISTRIBUTION:-unknown}}
LINUX_RELEASE=${LINUX_RELEASE:-${LINUX_BUILD_RELEASE:-unknown}}
end-of-config-file
	;;

config-makefile)
	cat <<end-of-config-makefile
end-of-config-makefile
	;;

*)
	echo "config/Linux.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
