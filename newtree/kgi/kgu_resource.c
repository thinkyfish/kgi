/* ---------------------------------------------------------------------------
**	Copyright (C)	1995-1997		Andreas Beck
**			1995-2000		Steffen Seeger
**	Copyright (C)	2002-2004           	Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ---------------------------------------------------------------------------
**
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>

int graph_resource_command(graph_file_t *file, unsigned int cmd, void *data)
{
	const kgic_resource_request_t *in = data;
	union {
		kgi_resource_t		common;
		kgi_clut_t		clut;
		kgi_tlut_t		tlut;
		kgi_marker_t		marker;
		kgi_text16_t		text16;

	} *r;

	if (! (file->flags & GRAPH_FF_CLIENT_IDENTIFIED)) {

		KRN_ERROR("cmd = %.8x, but client has not yet identified", cmd);
		return KGI_EPROTO;
	}

	/* check that the resource id is ok */
	if (((in->image == KGIC_MAPPER_NON_IMAGE_RESOURCE)?
	    __KGI_MAX_NR_IMAGE_RESOURCES:
	    __KGI_MAX_NR_RESOURCES) <= in->resource) {

		KRN_ERROR("invalid resource ID");
		return KGI_EINVAL;
	}

	/* check that the image number is ok */
	if ((in->image < 0) || (file->device->kgi.mode->images <= in->image)) {

		if (in->image != KGIC_MAPPER_NON_IMAGE_RESOURCE) {

			KRN_ERROR("invalid image %d", in->image);
			return KGI_EINVAL;
		}
	}

	/* get the mode resource or image resource */
	r = (in->image == KGIC_MAPPER_NON_IMAGE_RESOURCE) ?
	    (void*)file->device->kgi.mode->resource[in->resource] :
	    (void*)file->device->kgi.mode->img[in->image].resource[in->resource];

	if (NULL == r) {

		KRN_ERROR("no %s resource %d", 
		    in->image == KGIC_MAPPER_NON_IMAGE_RESOURCE?
		    "mode" : "image", in->resource);
		return KGI_EINVAL;
	}

	switch(cmd) {

	case KGIC_RESOURCE_CLUT_GET_INFO:
	{
		kgic_clut_get_info_result_t *out = data;
		
		if ((r->common.type != KGI_RT_ILUT_CONTROL) &&
		    (r->common.type != KGI_RT_ALUT_CONTROL))
			return KGI_EINVAL;
		
		out->tables = r->clut.tables;
		out->entries = r->clut.entries;
				
		return KGI_EOK;
	}
	
	case KGIC_RESOURCE_CLUT_SELECT:
	{
		kgic_clut_select_request_t *in = data;

		if ((r->common.type != KGI_RT_ILUT_CONTROL) &&
		    (r->common.type != KGI_RT_ALUT_CONTROL))
			return KGI_EINVAL;

		if (r->clut.Select) {
		
			(r->clut.Select)(&r->clut, in->lut);		
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}		

	case KGIC_RESOURCE_CLUT_SET:
	{
		kgic_clut_set_request_t *in = data;
		
		if ((r->common.type != KGI_RT_ILUT_CONTROL) &&
		    (r->common.type != KGI_RT_ALUT_CONTROL))
			return KGI_EINVAL;
		
		if (r->clut.Set) {
			kgi_u_t attrs;
			kgi_attribute_mask_t tam;

			if (in->idx >= r->clut.entries)
				return KGI_EINVAL;
				
			/* clamp the count */
			if (in->cnt + in->idx > r->clut.entries)
				in->cnt -= in->cnt + in->idx - r->clut.entries;

			/* count the number of attrs passed in for each entry */
			for (attrs = 0, tam = in->am; tam; tam >>= 1)
				attrs += (tam & 1) ? 1:0;
			
			if (attrs == 0)
				return KGI_EOK;
				
			(r->clut.Set)(&r->clut, in->lut, in->idx, in->cnt, 
				in->am, (kgi_u16_t*)in->data);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}	

	case KGIC_RESOURCE_TLUT_SELECT:
	{
		kgic_tlut_select_request_t *in = data;

		if (r->common.type != KGI_RT_TLUT_CONTROL)
			return KGI_EINVAL;

		if (r->tlut.Select) {

			(r->tlut.Select)(&r->tlut, in->lut);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}		

	case KGIC_RESOURCE_TLUT_SET:
	{
		kgic_tlut_set_request_t *in = data;

		if (r->common.type != KGI_RT_TLUT_CONTROL)
			return KGI_EINVAL;

		if (r->tlut.Set) {
			
			(r->tlut.Set)(&r->tlut, in->lut, in->idx, in->cnt, in->data);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}	

	case KGIC_RESOURCE_MARKER_SET_MODE:
	{
		kgic_marker_set_mode_request_t *in = data;

		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		if (r->marker.SetMode) {

			(r->marker.SetMode)(&r->marker, in->mode);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}	

	case KGIC_RESOURCE_MARKER_SELECT:
	{
		kgic_marker_select_request_t *in = data;

		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		if (r->marker.Select) {

			(r->marker.Select)(&r->marker, in->shape);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}

	case KGIC_RESOURCE_MARKER_SET_SHAPE:
	{
/*		kgic_marker_set_shape_request_t *in = data; */

		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;
		
/*		if (r->marker.SetShape) {
		
			COPY_DATA(in->data, 
			io_result = (r->marker.SetShape)(&r->marker, in->shape,
				in->hot_x, in->hot_y, data, */
		return KGI_EINVAL;
	}

	case KGIC_RESOURCE_MARKER_SHOW:
	{
		kgic_marker_show_request_t *in = data;

		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		if (r->marker.Show) {

			(r->marker.Show)(&r->marker, in->x, in->y);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}	

	case KGIC_RESOURCE_MARKER_HIDE:
	{
		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		if (r->marker.Hide) {

			(r->marker.Hide)(&r->marker);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}	

	case KGIC_RESOURCE_MARKER_UNDO:
	{
		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		if (r->marker.Undo) {

			(r->marker.Undo)(&r->marker);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}		

	/* this should probably be moved to KGI_MAPPER_RESOURCE_INFO */
	case KGIC_RESOURCE_MARKER_INFO:
	{
		kgic_marker_info_result_t *out = data;

		if ((r->common.type != KGI_RT_CURSOR_CONTROL) &&
		    (r->common.type != KGI_RT_POINTER_CONTROL))
			return KGI_EINVAL;

		out->modes  = r->marker.modes;
		out->shapes = r->marker.shapes;
		out->size   = r->marker.size;
		
		return KGI_EOK;
	}

#if 0
	case KGIC_RESOURCE_TEXT16_PUT_TEXT16:
	{
		kgic_text16_put_text16_request_t *in = data;

		if (r->common.type != KGI_RT_TEXT16_CONTROL)
			return KGI_EINVAL;

		if (r->text16.PutText16) {

			(r->text16.PutText16)(&r->text16, in->offset, 
				(kgi_u16_t*)in->data, in->cnt);
			return KGI_EOK;
		}
		return KGI_EINVAL;
	}		
#endif

	/* this should probably be moved to KGI_MAPPER_RESOURCE_INFO */
	case KGIC_RESOURCE_TEXT16_INFO:
	{
		kgic_text16_info_result_t *out = data;

		if (r->common.type != KGI_RT_TEXT16_CONTROL)
			return KGI_EINVAL;

		out->size = r->text16.size;
		out->virt = r->text16.virt;
		out->cell = r->text16.cell;
		out->font = r->text16.font;
			
		return KGI_EOK;
	}

	default:
		KRN_DEBUG(1, "unknown resource command %.4x", cmd);
		return KGI_ENXIO;
	}
}
