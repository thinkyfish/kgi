/*-
 * Copyright (c) 2002 Nicholas Souchu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and permit to persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,EXPRESSED OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
/*
 * KGI vidsw emulation driver
 */

/*
 * XXX
 * Currently this is only a complementary video_switch like VESA.
 * Later, it should be a true adapter/video driver like TGA/GFB coupled with
 * a syscons renderer.
 * XXX
 */

#include <kgi/maintainers.h>
#define	MAINTAINER	Nicholas_Souchu

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/vgareg.h>

#define	DEBUG_LEVEL	2

#define	KGI_SYS_NEED_IO
#include <kgi/system.h>
#include <kgi/debug.h>

#include <kgi/kgii.h>
#include <kgi/kgi.h>

static int kgia_init_done = 0;

static video_switch_t *prevvidsw;
static video_adapter_t *kgia_adp = NULL;

struct kgia_data
{
#ifdef kii_used
	kii_device_t		kii;
#endif
	kgi_device_t		kgi;
	kgi_mode_t		kgimode;

	kgi_mmio_region_t	*mmio;
	kgi_text16_t		*text16;
	kgi_ilut_t		*ilut;
	kgi_tlut_t		*tlut;
	kgi_marker_t		*cur;
	kgi_marker_t		*ptr;
	kgi_font_t		*font;

	kgi_ucoord_t		visible; /* visible size of buffer	*/
} kgia;

static int
kgia_error(void)
{

	return (1);
}

static int
kgia_probe(int unit, video_adapter_t **adpp, void *arg, int flags)
{

	return ((*prevvidsw->probe)(unit, adpp, arg, flags));
}

static int
kgia_init(int unit, video_adapter_t *adp, int flags)
{

	return ((*prevvidsw->init)(unit, adp, flags));
}

static int
kgia_get_info(video_adapter_t *adp, int mode, video_info_t *info)
{
	/* 
	 * XXX Bypass the KGI framework to provide directly the mode info XXX
	 */
	return ((*prevvidsw->get_info)(adp, mode, info));
}

static int
kgia_query_mode(video_adapter_t *adp, video_info_t *info)
{
	kgi_device_t k;
	kgi_mode_t m;

	memset(&m.img[0], 0, sizeof(info));
	m.images = 1;

	if (!(info->vi_flags & V_INFO_GRAPHICS))
		goto (error);

	m.img[0].fam |= KGI_AM_COLORS;

	switch (info->vi_depth) {
	case 16:			/* XXX only 16 bit modes */
	default:
		m.img[0].bpfa[0] = 5;
		m.img[0].bpfa[1] = 6;
		m.img[0].bpfa[2] = 5;
		break;
	}
	m.img[0].size.x = m.img[0].virt.x = info->vi_width;
	m.img[0].size.y = m.img[0].virt.y = info->vi_height;	
	
	k.mode = &m;

	if (kgi_register_device(&k, 0) != KGI_EOK)
		goto (error);

	kgi_unregister_device(&k);

	return (0);

 error:
	return (ENODEV);
}

static int
kgia_set_mode(video_adapter_t *adp, int mode)
{
	kgi_image_mode_t kgi_mode;

	/* XXX refuse other modes than 640x480x4 */
	if (mode != M_CG640x480)
		return (EINVAL);

	if (kgia.kgi.flags & KGI_DF_FOCUSED)
		kgi_unmap_device(kgia.kgi.id);

	/* XXX search mode according to mode index */
	memset(&kgi_mode, 0, sizeof(kgi_mode));
	kgi_mode.fam |= KGI_AM_COLORS;
	kgi_mode.cam |= KGI_AM_COLOR_INDEX;
	kgi_mode.bpfa[0] = 4;
	kgi_mode.bpfa[1] = 0;
	kgi_mode.size.x = kgi_mode.virt.x = 640;
	kgi_mode.size.y = kgi_mode.virt.y = 480;

	kgia.kgi.mode->img[0] = kgi_mode;

	kgi_map_device(kgia.kgi.id);

	return (0);
}

static int
kgia_save_font(video_adapter_t *adp, int page, int fontsize, u_char *data,
	       int ch, int count)
{

	if (!kgia.font)
		return (EINVAL);

	kgia.font->Save(kgia.font, page, fontsize, data, ch, count);

	return (0);
}

static int
kgia_load_font(video_adapter_t *adp, int page, int fontsize, u_char *data,
	       int ch, int count)
{

	if (!kgia.font)
		return (EINVAL);

	kgia.font->Load(kgia.font, page, fontsize, data, ch, count);

	return (0);
}

static int
kgia_show_font(video_adapter_t *adp, int page)
{

	if (!kgia.font)
		return (EINVAL);

	kgia.font->Show(kgia.font, page);

	return (0);
}

static int
kgia_save_palette(video_adapter_t *adp, u_char *palette)
{

	return (1);
}

static int
kgia_load_palette(video_adapter_t *adp, u_char *palette)
{

	return (1);
}

static int
kgia_set_border(video_adapter_t *adp, int color)
{

	return ((*prevvidsw->set_border)(adp, color));
}

static int
kgia_save_state(video_adapter_t *adp, void *p, size_t size)
{

	return ((*prevvidsw->save_state)(adp, p, size));
}

static int
kgia_load_state(video_adapter_t *adp, void *p)
{

	return ((*prevvidsw->load_state)(adp, p));
}

static int
kgia_set_origin(video_adapter_t *adp, off_t offset)
{

	if (!kgia.mmio)
		return (EINVAL);

	kgia.mmio->SetOffset(kgia.mmio, (kgi_size_t)offset);

	return (0);
}

static int
kgia_read_hw_cursor(video_adapter_t *adp, int *col, int *row)
{

	return (1);
}

static int
kgia_set_hw_cursor(video_adapter_t *adp, int col, int row)
{

	return (1);
}

static int
kgia_set_hw_cursor_shape(video_adapter_t *adp, int base, int height,
			 int celsize, int blink)
{

	return (1);
}

static int
kgia_blank_display(video_adapter_t *adp, int mode) 
{

	return ((*prevvidsw->blank_display)(adp, mode));
}

static int
kgia_mmap(video_adapter_t *adp, vm_offset_t offset, int prot)
{

	return (1);
}

static int
kgia_clear(video_adapter_t *adp)
{

	return (1);
}

static int
kgia_fill_rect(video_adapter_t *adp, int val, int x, int y, int cx, int cy)
{

	return (1);
}

static int
kgia_bitblt(video_adapter_t *adp,...)
{

	return (1);
}

static int
kgia_ioctl(video_adapter_t *adp, u_long cmd, caddr_t arg)
{

	return (1);
} 

static int
kgia_diag(video_adapter_t *adp, int level)
{

	return (1);
}

static video_switch_t kgividsw = {
	kgia_probe,
	kgia_init,
	kgia_get_info,
	kgia_query_mode,
	kgia_set_mode,
	kgia_save_font,
	kgia_load_font,
	kgia_show_font,
	kgia_save_palette,
	kgia_load_palette,
	kgia_set_border,
	kgia_save_state,
	kgia_load_state,
	kgia_set_origin,
	kgia_read_hw_cursor,
	kgia_set_hw_cursor,
	kgia_set_hw_cursor_shape,
	kgia_blank_display,
	kgia_mmap,
	kgia_ioctl,
	kgia_clear,
	kgia_fill_rect,
	kgia_bitblt,
	kgia_error,
	kgia_error,
	kgia_diag,
};

static int
kgia_configure(int flags)
{
	int error = 0;
	int i;
	video_adapter_t *adp;

	kgi_u_t	index = 0;
	kgi_resource_t *resource;
	kgi_marker_t *marker;

	/*
	 * If the kgia module has already been loaded, abort loading 
	 * the module this time.
	 */
	for (i = 0; (adp = vid_get_adapter(i)) != NULL; ++i) {
		if (adp->va_flags & V_ADP_KGI)
			return (ENXIO);
		if (adp->va_type == KD_VGA)
			break;
	}

	/*
	 * The VGA adapter is not found.  This is because either 
	 * 1) the VGA driver has not been initialized, or 2) the VGA card
	 * is not present.  If 1) is the case, we shall defer
	 * initialization for now and try again later.
	 */

	/* XXX may collide with VESA */
	if (adp == NULL) {
		vga_sub_configure = kgia_configure;
		return (ENODEV);
	}

	kgia_adp = adp;
	kgia_adp->va_flags |= V_ADP_KGI;

	/* Check that a kgi display is registered */
	if (kgi_display_registered(0) != KGI_EOK) {
		error = (ENODEV);
		goto end;
	}

	/* Register the adapter as a KGI device */
	if ((error = kgi_register_device(&(kgia.kgi), 0)) != KGI_EOK) 
		goto end;

	while ((index < __KGI_MAX_NR_IMAGE_RESOURCES) &&
	       (resource = kgia.kgi.mode->img[0].resource[index])) {		
		switch (resource->type) {			
		case KGI_RT_MMIO_FRAME_BUFFER:
			KRN_DEBUG(2, "mmio: %s", resource->name);
			kgia.mmio = (kgi_mmio_region_t *) resource;
			break;			
		case KGI_RT_TEXT16_CONTROL:
			KRN_DEBUG(2, "text16: %s", resource->name);
			kgia.text16 = (kgi_text16_t *) resource;
			break;			
		case KGI_RT_CURSOR_CONTROL:
			marker = (kgi_marker_t *) resource;
			if ((NULL == kgia.cur) ||
			    (kgia.cur->Undo && (NULL == marker->Undo))) {				
				KRN_DEBUG(2, "cursor: %s", resource->name);
				kgia.cur = marker;
			}
			break;			
		case KGI_RT_POINTER_CONTROL:
			marker = (kgi_marker_t *) resource;
			if ((NULL == kgia.ptr) ||
			    (kgia.ptr->Undo && (NULL == marker->Undo))) {				
				KRN_DEBUG(2, "pointer: %s", resource->name);
				kgia.ptr = marker;
			}
			break;			
		case KGI_RT_TLUT_CONTROL:
			KRN_DEBUG(2, "tlut: %s", resource->name);
			kgia.tlut = (kgi_tlut_t *) resource;
			break;			
		case KGI_RT_ILUT_CONTROL:
			KRN_DEBUG(2, "ilut: %s", resource->name);
			kgia.ilut = (kgi_ilut_t *) resource;
			break;
		case KGI_RT_FONT_CONTROL:
			KRN_DEBUG(2, "font: %s", resource->name);
			kgia.font = (kgi_font_t *) resource;
			break;
		default:
			break;
		}
		index++;
	}
	
	if (! (kgia.text16)) {
		KRN_DEBUG(1, "could not get text16 resource "
			  "(text16 %p, cur %p, ptr %p)",
			  kgia.text16, kgia.cur, kgia.ptr);
		return (EINVAL);
	}

	kgia.visible.x = kgia.text16->size.x;
	kgia.visible.y = kgia.text16->size.y;

	kgia.kgimode.images = 1;
	kgia.kgimode.img[0].flags |= KGI_IF_TEXT16;

	kgia.kgi.mode = &(kgia.kgimode);
	kgia.kgi.flags |= KGI_DF_CONSOLE;
	kgia.kgi.priv.priv_ptr = &kgia;
	
#ifdef kii_used
	kgia.kii.tty = current->tty;
	kgia.kii.flags |= KII_DF_CONSOLE;
	kgia.kii.priv.priv_ptr = kgia;
#endif
	kgia.kgi.MapDevice = NULL;
	kgia.kgi.UnmapDevice = NULL;
	kgia.kgi.HandleEvent = NULL;

	if (!kgi_current_focus(kgia.kgi.dpy_id)) {
		kgi_map_device(kgia.kgi.id);
#ifdef kii_used
		if (! kii_current_focus(kgia.kgi.kii.focus_id))
			kii_map_device(kgia.kgi.kii.id);
#endif
	}

 end:
	if (error) {
		kgia_adp = NULL;
		return (error);
	}

	prevvidsw = vidsw[kgia_adp->va_index];
	vidsw[kgia_adp->va_index] = &kgividsw;
	kgia_init_done = TRUE;

	return (0);
}

static int
kgia_load(void)
{
	int error = 0;
	int s;

	if (kgia_init_done)
		return (0);

	/* locate a VGA adapter */
	s = spltty();
	kgia_adp = NULL;
	error = kgia_configure(0);
	splx(s);

	return (error);
}

static int
kgia_unload(void)
{
	int error = 0;
	int s;

	s = spltty();
	if (kgia_adp != NULL) {
#ifdef kii_used
		if (kgia.kii.flags & KII_DF_FOCUSED) 
			kii_unmap_device(kgia.kii.id);
#endif
		if (kgia.kgi.flags & KGI_DF_FOCUSED) 
			kgi_unmap_device(kgia.kgi.id);
#ifdef kii_used
		kii_unregister_device(&(kgia.kii));
#endif
		kgi_unregister_device(&(kgia.kgi));

		kgia_adp->va_flags &= ~V_ADP_KGI;
		vidsw[kgia_adp->va_index] = prevvidsw;
	}
	splx(s);

	return (error);
}

static int
kgia_mod_event(module_t mod, int type, void *data)
{
	switch (type) {
	case MOD_LOAD:
		return (kgia_load());
	case MOD_UNLOAD:
		return (kgia_unload());
	default:
		break;
	}
	return (0);
}

static moduledata_t kgia_mod = {
	"kgia",
	kgia_mod_event,
	NULL,
};

DECLARE_MODULE(kgia, kgia_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
MODULE_VERSION(kgia, 1);

#include <kgi/kgi_version.h>
MODULE_DEPEND(kgia, kgi, KGI_MINVER, KGI_PREFVER, KGI_MAXVER);
MODULE_DEPEND(kgia, dpysw, 1, 1, 1);