/* ----------------------------------------------------------------------------
**	##VENDOR## ##META## chipset binding definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	##AUTHOR_NAME##
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ##META##-bind.h,v $
*/
#ifndef	_chipset_##VENDOR##_##META##_bind_h
#define	_chipset_##VENDOR##_##META##_bind_h

#include "chipset/##VENDOR##/##META##-meta.h"

KGIM_META(##meta##_chipset)
KGIM_META_INIT_MODULE_FN(##meta##_chipset)
KGIM_META_DONE_MODULE_FN(##meta##_chipset)

#endif	/* #ifdef _chipset_##VENDOR##_##META##_bind_h */
