/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## PLL binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ##META##-bind.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		##AUTHOR##
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#warning include register definitions here.

kgi_error_t ##meta##_clock_init_module(##meta##_clock_t *##meta##,
	##meta##_clock_io_t *##meta##_io,
	const kgim_options_t *options)
{
	kgim_strcpy(##meta##->pll.clock.vendor, "##VENDOR##");
	kgim_strcpy(##meta##->pll.clock.model, "##MODEL##");

	##meta##->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	##meta##->pll.clock.flags	*/
	##meta##->pll.clock.mode_size	= sizeof(##meta##_clock_mode_t);
/*	##meta##->pll.clock.Command	*/

	##meta##->pll.clock.type	= KGIM_CT_PROG;
#warning set DCLK range here.
	##meta##->pll.clock.dclk.range[0].min =    400000;
	##meta##->pll.clock.dclk.range[0].max = 120000000;

#warning set external PLL reference clock frequency here.
	##meta##->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);

#warning set PLL multiplier parameters here.
	##meta##->pll.a.mul = ##meta##->pll.a.div = 1;

#warning set VCO frequency limits here.
	##meta##->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min,  50000000);
	##meta##->pll.fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 120000000);

#warning set VCO post-divider range here.
	##meta##->pll.p.min = 0;
	##meta##->pll.p.max = 3;		/* post-divider range	*/
#warning set divident range here.
	##meta##->pll.mul.min = 1;
	##meta##->pll.mul.max = 2;		/* dividend range	*/
#warning set divisor range here.
	##meta##->pll.div.min = 1;
	##meta##->pll.div.max = 2;		/* divisor range	*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		##meta##->pll.clock.vendor, ##meta##->pll.clock.model);

	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		##meta##->pll.fref, 
		##meta##->pll.fvco.min, ##meta##->pll.fvco.max,
		options->clock->fref, options->clock->fvco_min,
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t ##meta##_clock_meta =
{
	(kgim_meta_init_module_fn *)	##meta##_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		##meta##_clock_init,
	(kgim_meta_done_fn *)		##meta##_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	##meta##_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,

	sizeof(##meta##_clock_t),
	0,
	sizeof(##meta##_clock_mode_t)
};
