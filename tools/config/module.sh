#
#	Configuration test Module
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--module-option] [--module-arg=<arg>]"
	;;

help-options)
	cat <<end-of-options
	--module-flag
		optional, default: not set
		This is a module option with no parameters.

	--module-arg=<arg>
		optional, default: "" (empty string)
		This is a module option.

end-of-options
	;;

config-parse)
	echo "Configuring module"
	for ARG; do case $ARG in

	--module-flag)
		MODULE_FLAG="set"
		;;

	--module-arg=*)
		MODULE_ARG=`configure_arg $ARG`
		;;

	--module-*)
		echo "module.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
module (module.sh):
	MODULE_FLAG			a module flag
	MODULE_ARG			a module argument

end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
MODULE_FLAG=${MODULE_FLAG:-not-set}
MODULE_ARG=${MODULE_ARG:-arg-default}
end-of-config-file
	;;

config-makefile)
#	cat $DIR_TOOLS/module/rules
	;;

*)
	echo "module.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
