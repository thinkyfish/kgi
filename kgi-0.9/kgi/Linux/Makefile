# -----------------------------------------------------------------------------
#	Linux kernel tree build Makefile
# -----------------------------------------------------------------------------

default.exit: linux_build

all.exit: linux_build

clean.exit:
	if [ -f linux/Makefile ]; then make -C linux/ clean; fi

distclean.exit: clean.exit
	rm -f *.log
	(cd linux;					\
	for i in `find -type d`; do			\
		case $$i in				\
		.|*/drivers|*/kgi*|*/CVS)	;;	\
		*)	rm -rf $$i;		;;	\
		esac;					\
	done)
	for i in `cat remove.files`; do rm -f linux/$$i; done

diffs:
	./mkdiffs.sh

linux/.depend:
	make -C linux/ depend  2>&1 | tee make-depend.log

linux_build: linux/.depend
	make -C linux/ vmlinux 2>&1 | tee make-vmlinux.log
	make -C linux/ modules 2>&1 | tee make-modules.log
