#
#	Configuration test Module
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--awk-bin=<awk>]"
	;;

help-options)
	cat <<end-of-options
	--awk-bin=<awk>
		optional, default: awk
		Set the awk command to invoke.

end-of-options
	;;

config-parse)
	echo "Configuring module"
	for ARG; do case $ARG in

	--awk-bin=*)
		AWK_BIN=`configure_arg $ARG`
		;;

	--awk-*)
		echo "awk/configure.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
awk (awk/configure.sh):
	AWK_BIN				the AWK command to invoke.

end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
AWK_BIN=${AWK_BIN:-awk}
end-of-config-file
	;;

config-makefile)
#	cat $DIR_TOOLS/module/rules
	;;

*)
	echo "awk/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
