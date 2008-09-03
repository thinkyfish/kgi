/* ----------------------------------------------------------------------------
**	KGI rendering
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

#define KGI_SYS_NEED_ATOMIC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_kgirndr.h>
#include <dev/kgc/kgc_backgnd.h>

#include "render_if.h"

/*	color mode attribute to pixel value mapping. We care of the foreground
**	and background colors. The rest is just best effort.
*/
kgi_u_t kgirndr_atop_color(render_t r, kgi_u_t attr)
{
	register kgi_u_t val = (attr & KGI_CA_REVERSE)
		? (((attr & 0x0007) << 8) | ((attr & 0x0700) << 4))
		: (((attr & 0x0007) << 12) | (attr & 0x0700));

	if (attr & KGI_CA_BLINK) {

		val |= 0x8000;
	}

	if (attr & KGI_CA_BOLD) {

		val |= 0x0800;
	}

	return val;
}

kgi_u_t kgirndr_ptoa_color(render_t r, kgi_u_t val)
{
	return	  ((val & 0x8000) ? KGI_CA_BLINK : KGI_CA_NORMAL)
		| ((val & 0x0800) ? KGI_CA_BOLD  : KGI_CA_NORMAL)
		| ((val & 0x7000) >> 12) | (val & 0x0F00);
}

/*	monochrome attribute mapping. We care about the attributes possible
**	and ignore colors.
*/
kgi_u_t kgirndr_atop_mono(kgi_u_t attr)
{
	register kgi_u_t
	val = (attr & KGI_CA_UNDERLINE)
		? 0x0100	/* MDA can't do reverse underline :-( */
		: ((attr & KGI_CA_REVERSE) ? 0x7000 : 0x0700);

	if (attr & KGI_CA_BLINK) {

		val |= 0x8000;
	}

	if (attr & KGI_CA_BOLD) {

		val |= 0x0800;
	}

	return val;
}

kgi_u_t kgirndr_ptoa_mono(render_t r, kgi_u_t val)
{
	register kgi_u_t
	attr = ((val & 0x7700) == 0x0100)
		? KGI_CA_UNDERLINE
		: (val & 0x7000) ? KGI_CA_REVERSE : KGI_CA_NORMAL;

	if (val & 0x8000) {

		attr |= KGI_CA_BLINK;
	}

	if (val & 0x0800) {

		attr |= KGI_CA_BOLD;
	}

	return attr;
}

static void
kgirndr_parse_resource(kgirndr_meta *render, kgi_resource_t *resource)
{
	kgi_marker_t *marker;

	switch (resource->type) {

	case KGI_RT_MMIO_FRAME_BUFFER:
		KRN_DEBUG(2, "mmio fb: %s", resource->name);
		render->fb = (kgi_mmio_region_t *) resource;
		break;

	case KGI_RT_TEXT16_CONTROL:
		KRN_DEBUG(2, "text16: %s", resource->name);
		render->text16 = (kgi_text16_t *) resource;
		break;

	case KGI_RT_CURSOR_CONTROL:
		marker = (kgi_marker_t *) resource;
		if ((NULL == render->cur) ||
		    (render->cur->Undo && (NULL == marker->Undo))) {

			KRN_DEBUG(2, "cursor: %s", resource->name);
			render->cur = marker;
		}
		break;

	case KGI_RT_POINTER_CONTROL:
		marker = (kgi_marker_t *) resource;
		if ((NULL == render->ptr) ||
		    (render->ptr->Undo && (NULL == marker->Undo))) {

			KRN_DEBUG(2, "pointer: %s", resource->name);
			render->ptr = marker;
		}
		break;

	case KGI_RT_TLUT_CONTROL:
		KRN_DEBUG(2, "tlut: %s", resource->name);
		render->tlut = (kgi_tlut_t *) resource;
		break;

	case KGI_RT_ILUT_CONTROL:
		KRN_DEBUG(2, "ilut: %s", resource->name);
		render->ilut = (kgi_ilut_t *) resource;
		break;

	default:
		KRN_ERROR("unknown resource->type 0x%.8x", resource->type);
		break;
	}

	return;
} 

kgi_error_t kgirndr_init(render_t r)
{
	kgirndr_meta *render = kgc_render_meta(r);
	kgi_resource_t *resource;
	kgi_u_t	index;

	/* Parse global resources */
	for (index = 0; index < __KGI_MAX_NR_IMAGE_RESOURCES; index ++) {
		if ((resource = render->kgi.mode->resource[index]))
			kgirndr_parse_resource(render, resource);
	}

	/* Parse per-image resources, eventually overwritting
	 * global resources.
	 */
	for (index = 0; index < __KGI_MAX_NR_IMAGE_RESOURCES; index ++) {
		if ((resource = render->kgi.mode->img[0].resource[index]))
			kgirndr_parse_resource(render, resource);
	}

	return KGI_EOK;
}

void kgirndr_hide_gadgets(render_t r)
{
	kgirndr_meta *render = kgc_render_meta(r);
	kgi_console_t *cons = kgc_render_cons(r);

	if (cons->flags & KGI_CF_POINTER_SHOWN) {

		KRN_ASSERT(render->ptr);
		render->ptr->Hide(render->ptr);
		cons->flags &= ~KGI_CF_POINTER_SHOWN;
	}
	if (cons->flags & KGI_CF_CURSOR_SHOWN) {

		KRN_ASSERT(render->cur);
		render->cur->Hide(render->cur);
		cons->flags &= ~KGI_CF_CURSOR_SHOWN;
	}
}

void kgirndr_undo_gadgets(render_t r)
{
	kgirndr_meta *render = kgc_render_meta(r);
	kgi_console_t *cons = kgc_render_cons(r);

	if ((cons->flags & KGI_CF_POINTER_SHOWN) && render->ptr->Undo) {

		render->ptr->Undo(render->ptr);
		cons->flags &= ~KGI_CF_POINTER_SHOWN;
	}
	if ((cons->flags & KGI_CF_CURSOR_SHOWN) && render->cur->Undo) {

		render->cur->Undo(render->cur);
		cons->flags &= ~KGI_CF_CURSOR_SHOWN;
	}
}

void kgirndr_show_gadgets(render_t r, kgi_u_t x, kgi_u_t y, kgi_u_t offset)
{
	kgirndr_meta *render = kgc_render_meta(r);
	kgi_console_t *cons = kgc_render_cons(r);

	kgirndr_undo_gadgets(r);

	if ((render->kgi.flags & KGI_DF_FOCUSED) &&
		(cons->kii.flags & KII_DF_FOCUSED) && !offset) {

		if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR) && render->cur) {

			cons->flags |= KGI_CF_CURSOR_SHOWN;
			render->cur->Show(render->cur, x, y);
		}
		if (CONSOLE_MODE(cons, KGI_CM_SHOW_POINTER) && render->ptr) {

			cons->flags |= KGI_CF_POINTER_SHOWN;
			render->ptr->Show(render->ptr,
				cons->kii.ptr.x, cons->kii.ptr.y);
		}

	} else {

		KRN_ASSERT(! (cons->flags & KGI_CF_CURSOR_SHOWN));
		KRN_ASSERT(! (cons->flags & KGI_CF_POINTER_SHOWN));
	}
}

#ifdef notyet
void kgirndr_unmap(render_t r)
{
	kgirndr_meta *render = kgc_render_meta(r);

	render->flags &= ~KGI_RF_NEEDS_UPDATE;
}

void kgirndr_map(render_t r)
{
	kgirndr_meta *render = kgc_render_meta(r);

	render->flags |= KGI_RF_NEEDS_UPDATE;

	if (render->ilut) {

		if (render->ilut->Set) {

			(render->ilut->Set)(render->ilut, NULL, 
				    0, 256, KGI_AM_COLORS, (kgi_u16_t *)&render->palette);
		}
		if (render->ilut->Select) {

			(render->ilut->Select)(render->ilut, 0);
		}
	}
	
	kgirndr_meta *render = kgc_render_meta(r);
	kgi_console_t *cons = kgc_render_cons(r);

	if (render->tlut) {

		if (render->tlut->Set) {

			(render->tlut->Set)(render->tlut, 0, 
				0, render->font->info->positions - 1,
				render->font->data);
		}
		if (render->tlut->Select) {

			(render->tlut->Select)(render->tlut, 0);
		}
	}
	if (render->ptr && 
		(render->ptr->modes & KGI_MM_3COLOR) &&
		(render->ptr->size.x == 64 && render->ptr->size.y == 64)) {

		(render->ptr->SetMode)(render->ptr, KGI_MM_3COLOR);
		(render->ptr->SetShape)(render->ptr, 0, 
			0,0, default_ptr_64x64, default_ptr_color);
		if (render->ptr->Select) {

			(render->ptr->Select)(render->ptr, 0);
		}
	}
}
#endif /* notyet */
