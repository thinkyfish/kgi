/* ----------------------------------------------------------------------------
**	declarations needed from xf86.h
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	xf86.h was Copyright (c) 1997 by The XFree86 Project, Inc.
*/
#ifndef _XF86_H
#define _XF86_H

#include "config.h"

#include <X11/Xfuncproto.h>
#include <stdarg.h>

#include "xf86str.h"
#include "xf86Opt.h"

/*	General parameters
*/
extern ScrnInfoPtr *xf86Screens;	/* List of pointers to ScrnInfoRecs */
extern const unsigned char byte_reversed[256];

void xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...);
void xf86ErrorF(const char *format, ...); 

#endif /* #ifndef _XF86_H */
