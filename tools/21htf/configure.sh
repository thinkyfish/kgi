#
#	htf2html (HyperText Fragment to HTML) generation tools
#
case $CONFIG_ACTION in

help-syntax)
	echo -n ""
	;;

help-options)
	cat <<end-of-options
end-of-options
	;;

config-parse)
	for ARG; do case $ARG in

	--htf-*)
		echo "module.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
end-of-config-file
	;;

config-makefile)
	cat $DIR_TOOLS/??htf/rules
	;;

*)
	echo "module.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
