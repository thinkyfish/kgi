Board(Matrox, G200_AGP)
#if Data
Begin
	SubsystemID(0x102B, 0x0521)
	Vendor("Matrox Graphics Inc")
	Model("G200 AGP")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx00, mgag_clock)
End
#endif
