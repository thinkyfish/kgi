OBJECTS = MDA.o VGA.o SVGA.o

%.o: %-timings.o monosync-meta.o monosync-bind.o
	$(LD) -o $@ -r $^

#######	timing sets are generated out of timing files
#
%-timings.c: %.t
	cat $^ | $(AWK_BIN) -f t2c.awk timingset=$* > $@

SVGA-timings.c: SVGA.t VGA.t

#######	common stuff
#
all.entry:: $(OBJECTS)

distclean.entry:: $(patsubst %.t,%-timings.c,$(wildcard *.t))

maintainer-clean.entry::
	rm -f *-timings.c

ifeq ($(CONFIG_MONITOR_DRIVER),monosync)
default.entry:: $(CONFIG_MONITOR_TIMING).o
endif
