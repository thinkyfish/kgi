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
	cat <<end-of-config-help
GNU make options (gmake/configure.sh)
	GMAKE_CLEAN_INIT		files removed for target clean.init
	GMAKE_REALCLEAN_INIT		files removed for target realclean.init
	GMAKE_DISTCLEAN_INIT		files removed for target distclean.init
end-of-config-help
	;;

config-file)
	;;

config-makefile)
	cat $DIR_TOOLS/??gmake/rules
	;;

*)
	echo "make/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
