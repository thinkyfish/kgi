Monitor(MAXDATA, Belinea108050, MAXDATA_Belinea108050)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("MAXDATA")
	Model("Belinea 10 80 50")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
/* ???	21" monitor */
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 200000000)
	hFreq(0, 30000, 95000)
	vFreq(0, 50, 160)
End
#endif
