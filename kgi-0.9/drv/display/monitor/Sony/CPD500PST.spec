Monitor(Sony, CPD500PST, Sony_CPD500PST)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("Multiscan CPD-500PST")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1280)
	Size(404, 302)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_VESA_DPMS |
		KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 250000000)
	hFreq(0, 30000, 107000)
	vFreq(0, 48, 160)
End
#endif
