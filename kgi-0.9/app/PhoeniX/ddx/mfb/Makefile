default.exit::	../mfb.so

OBJECTS = mfbgc.o	\
	mfbwindow.o	\
	mfbfont.o	\
	mfbfillrct.o	\
	mfbpntwin.o	\
	maskbits.o	\
	mfbpixmap.o	\
	mfbimage.o	\
	mfbline.o	\
	mfbbres.o	\
	mfbhrzvert.o	\
	mfbbresd.o	\
	mfbseg.o	\
	mfbpushpxl.o	\
	mfbzerarc.o	\
	mfbfillarc.o	\
	mfbfillsp.o	\
	mfbsetsp.o	\
	mfbscrinit.o	\
	mfbscrclse.o	\
	mfbclip.o	\
	mfbbitblt.o	\
	mfbgetsp.o	\
	mfbpolypnt.o	\
	mfbbltC.o	\
	mfbbltX.o	\
	mfbbltCI.o	\
	mfbbltO.o	\
	mfbbltG.o	\
	mfbpgbwht.o	\
	mfbpgbblak.o	\
	mfbpgbinv.o	\
	mfbigbwht.o	\
	mfbigbblak.o	\
	mfbcmap.o	\
	mfbpawhite.o	\
	mfbpablack.o	\
	mfbpainv.o	\
	mfbtileC.o	\
	mfbtileG.o	\
	mfbtewhite.o	\
	mfbteblack.o	\
	mfbmisc.o	\
	mfbbstore.o	\
	mfbplywhite.o	\
	mfbplyblack.o	\
	mfbplyinv.o

# SpecialCObjectRule(mfbpixmap,$(ICONFIGFILES),$(_NOOP_))

../mfb.so: ../mfb.o

../mfb.o: $(OBJECTS)
	$(LD) -r -o $@ $^
