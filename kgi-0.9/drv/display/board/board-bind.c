/* ----------------------------------------------------------------------------
**	KGI-0.9 module board binding driver
** ----------------------------------------------------------------------------
**	Copyright (c)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: board-bind.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#define	MODULE
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>

#include "kgi/module.h"

/* ----------------------------------------------------------------------------
**	module options
** ----------------------------------------------------------------------------
*/

/*	misc options
*/
signed int display = -1;	MODULE_PARM(display,  "0-4294967295i");
unsigned long law_base = 0;	MODULE_PARM(law_base, "1-4294967295i");

/*	PCI options
*/
signed int  pcibus = -1;	MODULE_PARM(pcibus, "0-255i");
signed int  pcidev = -1;	MODULE_PARM(pcidev, "0-255i");
signed int  pcifn  = -1;	MODULE_PARM(pcifn,  "0-255i");
unsigned long pcibase0 = 0;	MODULE_PARM(pcibase0, "1-4294967295i");
unsigned long pcibase1 = 0;	MODULE_PARM(pcibase1, "1-4294967295i");
unsigned long pcibase2 = 0;	MODULE_PARM(pcibase2, "1-4294967295i");
unsigned long pcibase3 = 0;	MODULE_PARM(pcibase3, "1-4294967295i");
unsigned long pcibase4 = 0;	MODULE_PARM(pcibase4, "1-4294967295i");
unsigned long pcibase5 = 0;	MODULE_PARM(pcibase5, "1-4294967295i");
unsigned long pcibase6 = 0;	MODULE_PARM(pcibase6, "1-4294967295i");
unsigned long pcibase7 = 0;	MODULE_PARM(pcibase7, "1-4294967295i");

/*	DAC options
*/
unsigned int dac_lclk_min =  0;	MODULE_PARM(dac_lclk_min, "0-4294967295i");
unsigned int dac_lclk_max = -1;	MODULE_PARM(dac_lclk_max, "0-4294967295i");
unsigned int dac_dclk_min =  0;	MODULE_PARM(dac_dclk_min, "0-4294967295i");
unsigned int dac_dclk_max = -1;	MODULE_PARM(dac_dclk_max, "0-4294967295i");

/*	Clock options
*/
unsigned int fref =  0;		MODULE_PARM(fref, "0-4294967295i");
unsigned int fvco_min = 0;	MODULE_PARM(fvco_min, "0-4294967295i");
unsigned int fvco_max = 0;	MODULE_PARM(fvco_max, "0-4294967295i");

/*	chipset options
*/
char *chipset = NULL;		MODULE_PARM(chipset, "s");
unsigned int chipset_ram = 0;	MODULE_PARM(chipset_ram, "0-4294967295i");

/*	Initialize kgim_display_t.options from the module-parameters passed.
**	This is common to all boards but needs to be local to each module.
*/
static inline void kgim_options_init(kgim_display_t *dpy, 
	const kgi_u32_t *subsystemID)
{
	KRN_DEBUG(2, "kgim_options_init()");
	
	/*
	**	enter options
	*/
	dpy->options_misc.display  = display;
	dpy->options_misc.law_base = law_base;
	dpy->options.misc = &(dpy->options_misc);

	if (((pcibus != -1) || (pcidev != -1)) && (pcifn == -1)) {

		pcifn = 0;
	}
	dpy->options_pci.dev = PCICFG_VADDR(pcibus, pcidev, pcifn);
	if (PCICFG_NULL == dpy->options_pci.dev) {

		KRN_DEBUG(2, "probing subsystem");
		if (pcicfg_find_subsystem(&dpy->options_pci.dev, subsystemID)) {

			KRN_DEBUG(1, "subsystem autodetect failed.");
			dpy->options_pci.dev = PCICFG_NULL;
		}
	}
	dpy->options_pci.base0 = pcibase0;
	dpy->options_pci.base1 = pcibase1;
	dpy->options_pci.base2 = pcibase2;
	dpy->options_pci.base3 = pcibase3;
	dpy->options_pci.base4 = pcibase4;
	dpy->options_pci.base5 = pcibase5;
	dpy->options_pci.base6 = pcibase6;
	dpy->options_pci.base7 = pcibase7;
	dpy->options.pci = &(dpy->options_pci);

	dpy->options_chipset.chipset = chipset;
	dpy->options_chipset.memory = chipset_ram KB;
	dpy->options.chipset = &(dpy->options_chipset);

	dpy->options_ramdac.lclk_min = dac_lclk_min;
	dpy->options_ramdac.lclk_max = dac_lclk_max;
	dpy->options_ramdac.dclk_min = dac_dclk_min;
	dpy->options_ramdac.dclk_max = dac_dclk_max;
	dpy->options.ramdac = &(dpy->options_ramdac);

	dpy->options_clock.fref = fref;
	dpy->options_clock.fvco_min = fvco_min;
	dpy->options_clock.fvco_max = fvco_max;
	dpy->options.clock = &(dpy->options_clock);
}


/* ----------------------------------------------------------------------------
**	board- and module-specific functions and declarations.
** ----------------------------------------------------------------------------
**	These are derived from the spec file misusing CPP. Consider it 
**	'advanced voodoo' if you like.
*/
#define	Data	1
#define	Board(vendor, model)	\
	static kgim_display_t kgim_##vendor##_##model##_display;	\
									\
	static void kgim_##vendor##_##model##_inc_refcount(kgi_display_t *dpy) \
	{								\
		MOD_INC_USE_COUNT;					\
	}								\
									\
	static void kgim_##vendor##_##model##_dec_refcount(kgi_display_t *dpy) \
	{								\
		MOD_DEC_USE_COUNT;					\
	}								\
									\
	static const kgi_u32_t kgim_##vendor##_##model##_subsystemID[] = \
	{

#define	Begin

#define	SubsystemID(vendor, device)					\
		PCICFG_SIGNATURE(vendor, device),			\

#define	Vendor(vendor_name)						\
		PCICFG_SIGNATURE(0x0000, 0x0000)			\
	};

#define	Model(model_name)

#define	Driver(subsystem, driver, meta)	\
	extern kgim_meta_t	meta##_meta;

#define	End

#ifndef	BOARD_SPEC
#	define	BOARD_SPEC	"board-bind.spec"
#endif

#include BOARD_SPEC

#undef	Board
#undef	Data
#undef	Begin
#undef	SubsystemID
#undef	Vendor
#undef	Model
#undef	Driver
#undef	End

/*	For now we hardcode the monosync driver here.
*/
extern kgim_meta_t	monosync_monitor_meta;

/*	Now derive the cleanup_module() and init_module() functions.
**	Techniques used are as above.
*/
#define	Board(vendor, model)						\
	void cleanup_module(void)					\
	{								\
		kgim_display_t *dpy = &kgim_##vendor##_##model##_display; \
		kgim_display_done(dpy);					\
	}								\
									\
	int init_module(void)						\
	{								\
		kgim_display_t *dpy = &kgim_##vendor##_##model##_display; \
		const kgi_u32_t *subsystemID =				\
			 kgim_##vendor##_##model##_subsystemID;		\
		kgim_memset(dpy, 0, sizeof(*dpy));			\
		dpy->kgi.IncRefcount = kgim_##vendor##_##model##_inc_refcount;\
		dpy->kgi.DecRefcount = kgim_##vendor##_##model##_dec_refcount;

#define	Begin

#define	SubsystemID(vendor, device)

#define	Vendor(vendor_name)						\
		kgim_strcpy(dpy->kgi.vendor, vendor_name);

#define	Model(model_name)						\
		kgim_strcpy(dpy->kgi.model, model_name);

#define	Driver(sys, driver, meta)					\
		dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang = &meta##_meta;

#define	End								\
		dpy->subsystem[KGIM_SUBSYSTEM_monitor].meta_lang =	\
			&monosync_monitor_meta;				\
									\
		kgim_options_init(dpy, subsystemID);			\
		return kgim_display_init(dpy);				\
	}
#define	Data	1

#include BOARD_SPEC
