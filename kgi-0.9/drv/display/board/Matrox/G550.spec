Board(Matrox, G550)
#if Data
Begin
	SubsystemID(0x102B, 0x2527)
	Vendor("Matrox Graphics Inc")
	Model("G550")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx50, Gx50_clock)
End
#endif
