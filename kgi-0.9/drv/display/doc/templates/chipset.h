/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## chipset register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ##META##.h,v $
*/
#ifndef	_chipset_##VENDOR##_##META##_h
#define	_chipset_##VENDOR##_##META##_h

/*	PCI config region
**
**	Please use the pci.h definitions for PCICFG register names.
*/
#warning	add PCI vendor and model ID(s) here.
#ifndef	PCI_VENDOR_ID_##VENDOR##
#define	PCI_VENDOR_ID_##VENDOR##		0x0000
#endif
#ifndef	PCI_DEVICE_ID_##VENDOR##_##MODEL##
#define	PCI_DEVICE_ID_##VENDOR##_##MODEL##	0x0000
#endif

#warning	add PCI aperture sizes here.
#define	##META##_Base0_Size	0x10000
#define	##META##_Base1_Size	0x10000
#define	##META##_ROM_Size	0x10000

#warning	add chipset register definitions here
#define	##META##_MAX_DacRegisters	16

#endif	/* #ifdef _chipset_##VENDOR##_##META##_h	*/
