Board(S3, ViRGE)
#if Data
Begin
	Vendor("S3")
	Model("ViRGE")
	Driver(chipset,	S3/VIRGE,  virge_chipset)
	Driver(ramdac,	S3/VIRGE, virge_ramdac)
	Driver(clock,	S3/VIRGE, virge_clock)
End
#endif
