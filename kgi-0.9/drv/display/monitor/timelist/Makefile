OBJECTS = MDA.o VGA.o SVGA.o HPCCMO.o

%.o: %-timelist.o timelist-meta.o timelist-bind.o
	$(LD) -o $@ -r $^

#######	timing sets are generated out of timing files
#
%-timelist.c: %.t
	cat $^ | $(AWK_BIN) -f t2c.awk timelistet=$* > $@

SVGA-timelist.c: SVGA.t VGA.t

#######	common stuff
#
all.entry:: $(OBJECTS)

distclean.entry:: $(patsubst %.t,%-timelist.c,$(wildcard *.t))

maintainer-clean.entry::
	rm -f *-timelist.c

ifeq ($(CONFIG_MONITOR_DRIVER),timelist)
default.entry:: $(CONFIG_MONITOR_TIMING).o
endif
