
default.exit:: ../Xtrans.o


OBJECTS = trans-x11.o	\
	trans-font.o

../Xtrans.o: $(OBJECTS)
	$(LD) -r -o $@ $^

#	X11 server communication functions
#
FLAGS_X11  = -DXSERV_t -DTRANS_SERVER 

trans-x11.o: transport.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) $(FLAGS_X11) -c $< -o $@

#	X11 server font communication functions
#
FLAGS_FONT = -DFONT_t -DTRANS_SERVER -DTRANS_CLIENT -DTRANS_REOPEN

trans-font.o: transport.c
	$(CC) $(CC_OPT) $(CC_INCLUDE_DIRS) $(CFLAGS) $(FLAGS_FONT) -c $< -o $@
