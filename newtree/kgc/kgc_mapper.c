/* ----------------------------------------------------------------------------
**	KGI console mapper
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**	Copyright (C)	2003		Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_scroller.h>
#include <dev/kgc/kgc_render.h>

#include "scroller_if.h"
#include "render_if.h"

kii_s_t kgc_unmap_kii(kii_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->render)
		RENDER_HIDE_GADGETS((render_t)cons->render);

	return KII_EOK;
}

void kgc_map_kii(kii_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->render)
		RENDER_SHOW_GADGETS((render_t)cons->render, 0, 0, 0);
}

kgi_s_t kgc_unmap_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->scroller)
		SCROLLER_UNMAP((scroller_t)cons->scroller);
	if (cons && cons->render)
		RENDER_UNMAP((render_t)cons->render);

	return KGI_EOK;
}

void kgc_map_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	/* Render mapped before scroller to enable scroller sync
	 */
	if (cons && cons->render)
		RENDER_MAP((render_t)cons->render);
	if (cons && cons->scroller)
		SCROLLER_MAP((scroller_t)cons->scroller);
}
