/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## PLL implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ##META##-meta.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	##AUTHOR##
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/##VENDOR##/##META##.h"
#include "clock/##VENDOR##/##META##-meta.h"

#warning add I/O helper functions here.
/***	I/O helper functions may simplify access to the PLL, e.g. setting
****	extended (indexed) registers etc. This is, however, optional.
***/

kgi_error_t ##meta##_clock_init(##meta##_clock_t *##meta##, 
	##meta##_clock_io_t *##meta##_io, 
	const kgim_options_t *options)
{
#warning save the initial PLL state here (if applicable).

	return KGI_EOK;
}

void ##meta##_clock_done(##meta##_clock_t *##meta##,
	##meta##_clock_io_t *##meta##_io, 
	const kgim_options_t *options)
{
#warning restore the initial PLL state here (if applicable).
}

void ##meta##_clock_mode_enter(##meta##_clock_t *##meta##,
	##meta##_clock_io_t *##meta##_io,
	##meta##_clock_mode_t *##meta##_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
#warning assert yourself the parameters are valid here (hardcode limits!)
	KRN_ASSERT(##meta##_mode->pll.mul < 2);
	KRN_ASSERT(##meta##_mode->pll.div < 2);
	KRN_ASSERT(##meta##_mode->pll.p   < 3);

#warning do the clock programming here.
#warning wait until the PLL is locked here.
	/****	if this times out due to whatever reason, issue a warning
	*****	with the KRN_ERROR() function but return.
	****/
}
