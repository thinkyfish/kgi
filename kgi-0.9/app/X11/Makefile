# -----------------------------------------------------------------------------
#	X11 build Makefile
# -----------------------------------------------------------------------------

default.exit:
	(cd xc; make World 2>&1 | tee ../make-World.log)

clean.exit:
	if [ -f xc/Makefile && -f xc/xmakefile ]; then make -C xc/ clean; fi

distclean.exit: clean.exit
	rm -f *.log
	(cd xc;						\
	for i in `find -type d`; do			\
		case $$i in				\
		.|*/programs|*/Xserver|*/hw|*/xggi*|*/CVS)	;;	\
		*)	rm -rf $$i;		;;	\
		esac;					\
	done)
	for i in `cat remove.files`; do rm -f xc/$$i; done

diffs:
	./mkdiffs.sh
