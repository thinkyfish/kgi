
default.exit:	../ddx.o

OBJECTS = phoenixInput.o	\
	phoenixOutput.o		\
	x2unicode.o

../ddx.o: $(OBJECTS)
	$(LD) -r -o $@ $^

