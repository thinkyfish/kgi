
default.exit:: ../cfb.so ../cfb08.so ../cfb16.so ../cfb24.so ../cfb32.so

#
#	stems for shared sources
#

STEMS = cfbgc		\
	cfbrrop		\
	cfbwindow	\
	cfbgetsp	\
	cfbfillrct	\
	cfbsolidC	\
	cfbsolidX	\
	cfbsolidG	\
	cfbtileoddC	\
	cfbtileoddG	\
	cfbtile32C	\
	cfbtile32G	\
	cfbfillsp	\
	cfbsetsp	\
	cfbscrinit	\
	cfballpriv	\
	cfbpntwin	\
	cfbmskbits	\
	cfbpixmap	\
	cfbzerarcC	\
	cfbzerarcX	\
	cfbzerarcG	\
	cfbfillarcC	\
	cfbfillarcG	\
	cfbigblt8	\
	cfbglblt8	\
	cfbtegblt	\
	cfbbstore	\
	cfbpolypnt	\
	cfbbres		\
	cfbline		\
	cfbhrzvert	\
	cfbbresd	\
	cfbimage	\
	cfbseg		\
	cfb8lineCO	\
	cfb8lineX	\
	cfb8lineG	\
	cfb8lineCP	\
	cfb8segCS	\
	cfb8segX	\
	cfb8segG	\
	cfb8segC	\
	cfbbitblt	\
	cfbbltC		\
	cfbbltX		\
	cfbbltO		\
	cfbbltG		\
	cfbply1rctC	\
	cfbply1rctG	\
	cfbcppl

#
#	objects to link
#
OBJECTS_COMMON = cfbcmap.o
OBJECTS_CFB08 = $(patsubst %,cfb08-%.o,$(STEMS))
OBJECTS_CFB16 = $(patsubst %,cfb16-%.o,$(STEMS))
OBJECTS_CFB24 = $(patsubst %,cfb24-%.o,$(STEMS))
OBJECTS_CFB32 = $(patsubst %,cfb32-%.o,$(STEMS))

../cfb.o: $(OBJECTS_COMMON)
	$(LD) -r -o $@ $^

../cfb08.o: $(OBJECTS_CFB08) \
	cfb8bit.o	\
	cfbteblt8.o	\
	cfbglrop8.o	\
	cfbpush8.o	\
	cfbrctstp8.o
	$(LD) -r -o $@ $^

../cfb16.o: $(OBJECTS_CFB16)
	$(LD) -r -o $@ $^

../cfb24.o: $(OBJECTS_CFB24)
	$(LD) -r -o $@ $^

../cfb32.o: $(OBJECTS_CFB32)
	$(LD) -r -o $@ $^

#
#	rules for shared objects
#
../cfb.so: ../cfb.o

../cfb08.so: ../cfb08.o

../cfb16.so: ../cfb16.o

../cfb24.so: ../cfb24.o

../cfb32.so: ../cfb32.o

#
#	implicit rules to build objects
#

cfb08-%.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DPSZ=8 -c $< -o $@

cfb16-%.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DPSZ=16 -c $< -o $@

cfb24-%.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DPSZ=24 -c $< -o $@

cfb32-%.o: %.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) -DPSZ=32 -c $< -o $@

