#
#	GNU make configuration
#
case $CONFIG_ACTION in

help-syntax)
	;;

help-options)
	;;

config-parse)
	;;

config-help)
	;;

config-file)
	;;

config-makefile)
	cat $DIR_TOOLS/??gmake/rules
	;;

*)
	echo "make/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
