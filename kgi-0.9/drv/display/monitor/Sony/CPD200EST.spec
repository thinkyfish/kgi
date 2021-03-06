Monitor(Sony, CPD200EST, Sony_CPD200EST)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("Multiscan CPD-200EST")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1280, 1024)
	Size(328, 242)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 135000000)
	hFreq(0, 30000, 70000)
	vFreq(0, 50, 120)
End
#endif
