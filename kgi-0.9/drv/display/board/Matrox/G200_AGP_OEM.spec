Board(Matrox, G200_AGP_OEM)
#if Data
Begin
	SubsystemID(0x102B, 0x0521)
	Vendor("Matrox Graphics Inc")
	Model("G200 AGP (OEM version)")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx00, mgag_clock)
End
#endif
