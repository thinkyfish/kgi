COMMENT	KGI Monitor Specification Template file.
COMMENT
COMMENT	If your Monitor is not yet supported, please check your monitor
COMMENT	manual if it contains a technical specification. If it does, please
COMMENT	complete this form and e-mail it to <seeger@physik.tu-chemnitz.de>.
COMMENT	Please remove this comment and all COMMENT/MANDATORY/OPTIONAL lines. 
COMMENT	Please double-check the MANDATORY sections are correct and completed.
COMMENT	Otherwise your contribution may be rejected (silently!).
COMMENT
COMMENT	Thank you very much for your contribution.
COMMENT
COMMENT	If you want to make comments, you may use standard ANSI C comments.
COMMENT
COMMENT --- begin spec file --------------------------------------------------
MANDATORY	A single-word short name for this driver. These must be valid
MANDATORY	C-identifers (e.g. contain only [a-z,A-Z,0-9,'_').
Monitor(Vendor-short-name, Model-short-name, Meta-language-name)
#if Data
Begin
MANDATORY	Please enter your name and e-mail address here. 
	Contributor("Your Name <your.name@yourdomain.dom>")

OPTIONAL	You may want to comment where this data came from. 

MANDATORY	Who built this device?
	Vendor("Vendor Full Name")

MANDATORY	What's the marketing departments name for this device?
	Model("Model Full Name")

OPTIONAL	You may ommit this if <flags> is KGIM_MF_NORMAL. */
OPTIONAL	Valid arguments are a binary <or> of KGIM_MF_ constants (see
OPTIONAL	../kgi/module.h for details!
	Flags(KGIM_MF_NORMAL)

MANDATORY	Give the maximum resoulution supported.
	MaxRes(1600, 1200)

OPTIONAL	Give the size of the visible area in mm
	Size(360, 270)

MANDATORY	Give the display technology used. (Please see ../kgi/module.h)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)

MANDATORY	Synchronization types _supported_. (Please see ../kgi/module.h)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)

MANDATORY	Bandwidth (recommended DCLK range) in Hz
	Bandwidth(minimum_dclk, maximum_dclk)

MANDATORY	This applies to both the hFreq and vFreq parameters.
MANDATORY	At least one range must be given. Valid range_number's are 0-7.
MANDATORY	If multiple ranges are given, they must be in ascending (range
MANDATORY	and frequency order and must not be overlapping in frequencies.
MANDATORY	hFreq and vFreq ranges are independent.
	hFreq(range_number, minimum_hfreq, maximum_hfreq)
	vFreq(range_number, minimum_vfreq, maximum_vfreq)
End
#endif
