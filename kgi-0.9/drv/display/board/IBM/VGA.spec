Board(IBM, Generic_VGA_card)
#if Data
Begin
	Vendor("Industrial Business Machines Inc.")
	Model("VGA")
	Driver(chipset,	IBM/VGA, vga_chipset)
	Driver(ramdac,	IBM/VGA, vga_ramdac)
	Driver(clock,	IBM/VGA, fixed_clock)
End
#endif
