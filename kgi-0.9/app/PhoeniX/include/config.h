
#ifndef	_config_h
#define _config_h

#undef	NeedFunctionPrototypes

#define	PIXMAP_PRIVATES

/* ifdef BuildLowMem
**	#define	LOWMEMFTPT
*/

/* ----------------------------------------------------------------------------
**	extensions
** ----------------------------------------------------------------------------
*/

#undef	X11_EXT_DEC_DPMS
#undef	X11_EXT_DEC_XINERAMA

#undef	X11_EXT_NCD_LBX

#undef	X11_EXT_TOG_SYNC
#undef	X11_EXT_TOG_BIGREQUESTS
#undef	X11_EXT_TOG_XINPUT
#undef	X11_EXT_TOG_XTEST

#undef	X11_EXT_SGI_MISC
#undef	X11_EXT_SGI_XKB

#undef	X11_EXT_XC_APPGROUP
#undef	X11_EXT_XC_SECURITY
#undef	X11_EXT_XC_XPRINT

#undef	X11_EXT_XFREE86_BIGFONT


/* ----------------------------------------------------------------------------
**	connections
** ----------------------------------------------------------------------------
*/

#undef	X11_CONN_AMTCP
#undef	X11_CONN_AMRPC
#undef	X11_CONN_CHAOS
#undef	X11_CONN_DNET
#undef	X11_CONN_LOCAL
#define	X11_CONN_TCP		y
#define	X11_CONN_UNIX		y
#undef	X11_CONN_STREAMS



#define	USE_RGB_TXT


#include "compatibility.h"

#endif	/* #undef _config_h	*/
