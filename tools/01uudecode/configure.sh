#
#	uudecode configuration
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--uudecode-bin=<uudecode>]"
	;;

help-options)
	cat <<end-of-options
	--uudecode=<uudecode>
		optional, default: uudecode
		uudecode binary to use.

end-of-options
	;;

config-parse)
	for ARG; do case $ARG in

	--uudecode-bin=*)
		UUENCODE=`configure_arg $ARG`
		;;

	--uudecode-*)
		echo "uudecode/configure.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
uudecode (uudecode/configure.sh):
	UUDECODE			uudecode binary to use

end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
UUDECODE=${UUDECODE:-uudecode}
end-of-config-file
	;;

config-makefile)
	cat $DIR_TOOLS/??uudecode/rules
	;;

*)
	echo "uudecode/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
