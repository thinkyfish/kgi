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

#define	xf86ScreenPtr	ScrnInfoPtr
#define	xf86ScreenRec	ScrnInfoRec

/*	General parameters
*/
extern int xf86PrivateIndex;
extern const unsigned char byte_reversed[256];

#define	XF86_SCREEN_PTR(pScreen) ((xf86ScreenPtr) (pScreen->devPrivates[xf86PrivateIndex].ptr))

extern xf86ScreenPtr xf86CreateScreenInfoRec(void);
extern Bool xf86Init(ScreenPtr screen, xf86ScreenPtr xf86);
extern void xf86DestroyScreenInfoRec(xf86ScreenPtr xf86);

void xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...);
void xf86ErrorF(const char *format, ...); 

#endif /* #ifndef _XF86_H */
