/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## register definitions
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
#ifndef	_ramdac_##VENDOR##_##META##_h
#define	_ramdac_##VENDOR##_##META##_h

/***	We assume a VGA compatible DAC here. If is not, change the 'base'
****	register definitions below accordingly.
***/
#warning check/add definitions for direct DAC registers here.
#define ##META##_DAC_PW_INDEX		0x0
#define ##META##_DAC_P_DATA		0x1
#define ##META##_DAC_PIXEL_MASK		0x2
#define ##META##_DAC_PR_INDEX		0x3

#define ##META##_DAC_EXT_ADDR		0x0
#define ##META##_DAC_CW_INDEX		0x4
#define ##META##_DAC_C_DATA		0x5

/***	Add any indexed registers below.
****
***/
#warning check/add definitions for indirect DAC registers here.

#endif	/* #ifdef _ramdac_##VENDOR##_##META##_h	*/
