#
#	HTML language module
#
case $CONFIG_ACTION in

help-syntax)
	echo -n " [--html-languages=<lang>]"
	;;

help-options)
	cat <<end-of-options
	--html-languages=<lang-list>
		optional, default: en
		A space-separated list of the languages HTML documents shall be
		generated for. <lang> is a list of ISO-639.1 language 
		identifiers in the order of priority if the client has no 
		(matching) preference.

	--html-site-url=<url>
		optional, default: file:/$DIR_INSTALL/html
		The top level URL for the files 

	--html-site-name=<name>
		optional, default: "" (empty string)
		The site name for the files generated.

end-of-options
	;;

config-parse)
	for ARG; do case $ARG in

	--html-languages=*)
		HTML_LANGUAGES=`configure_arg $ARG`
		;;

	--html-site-url=*)
		HTML_SITE_URL=`configure_arg $ARG`
		;;

	--html-site-name=*)
		HTML_SITE_NAME=`configure_arg $ARG`
		;;

	--html-*)
		echo "html/configure.sh: unknown module option '$ARG'"
		exit 1
	esac; done
	;;

config-help)
	cat <<end-of-config-help
html (html/configure.sh):
	HTML_LANGUAGES			document languages to build
	HTML_SITE_URL			top-level URL of published pages
	HTML_SITE_NAME			the site name that publishes these pages

end-of-config-help
	;;

config-file)
	cat <<end-of-config-file
#
#	HTML options
#
HTML_LANGUAGES="${HTML_LANGUAGES:-en}"
HTML_SITE_URL="${HTML_SITE_URL:-file:$DIR_PREFIX/html}"
HTML_SITE_NAME="$HTML_SITE_NAME"
end-of-config-file
	;;

config-makefile)
	cat $DIR_TOOLS/??html/rules
	;;

*)
	echo "html/configure.sh: Warning: unknown configuration action '$CONFIG_ACTION'"
esac
