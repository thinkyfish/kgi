Board(Matrox, G200_PCI)
#if Data
Begin
	SubsystemID(0x102B, 0x0520)
	Vendor("Matrox Graphics Inc")
	Model("G200 PCI")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx00, mgag_clock)
End
#endif
