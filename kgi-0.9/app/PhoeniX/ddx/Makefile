
default.exit:	../ddx.o

OBJECTS = phoenixInput.o	\
	phoenixOutput.o		\
	kii.o			\
	kgi.o			\
	x2unicode.o

../ddx.o: $(OBJECTS)
	$(LD) -r -o $@ $^

