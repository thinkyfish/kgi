/* ----------------------------------------------------------------------------
**      IBM VGA (text mode only) meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-text-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:19  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif


#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-text-meta.h"

kgi_error_t vga_text_chipset_init(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, const kgim_options_t *options)
{
	kgi_u_t i;

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	kgim_memset(vga, 0, sizeof(*vga));
	kgim_strcpy(vga->chipset.vendor, "IBM");
	kgim_strcpy(vga->chipset.model, "VGA (text mode)");
	vga->chipset.revision	= KGIM_CHIPSET_REVISION;
	vga->chipset.flags	= KGIM_CF_NORMAL;
	vga->chipset.mode_size	= sizeof(vga_text_chipset_mode_t);

	vga->chipset.maxdots.x	= 0x3FF;
	vga->chipset.maxdots.y	= 0x3FF;
	vga->chipset.memory	= 256 KB;
	vga->chipset.dclk.min	= 10000000;
	vga->chipset.dclk.max	= 30000000;
	vga->textmemory		= 32 KB;

	vga->MISC = VGA_MISC_IN8(vga_io);

	vga->FCTRL = VGA_FCTRL_IN8(vga_io);

#define	VGA_SAVE(subsys)						\
		for (i = 0; i < VGA_##subsys##_REGS; i++) {		\
									\
			vga->subsys[i] = VGA_##subsys##_IN8(vga_io, i);	\
		}

		VGA_SAVE(SEQ);
		VGA_SAVE(CRT);
		VGA_SAVE(GRC);
		VGA_SAVE(ATC);
#undef	VGA_SAVE

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		vga->chipset.vendor, vga->chipset.model);
	return KGI_EOK;
}

void vga_text_chipset_done(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, const kgim_options_t *options)
{
	kgi_u_t i;

#define	VGA_RESTORE(subsys)						\
		for (i = 0; i < VGA_##subsys##_REGS; i++) {		\
									\
			VGA_##subsys##_OUT8(vga_io, vga->subsys[i], i);	\
		}

		VGA_RESTORE(SEQ);
		VGA_CRT_OUT8(vga_io,
			VGA_CRT_IN8(vga_io, 0x11) & ~VGA_CR11_LOCKTIMING, 0x11);
		VGA_RESTORE(CRT);
		VGA_RESTORE(GRC);
		VGA_RESTORE(ATC);

		VGA_FCTRL_OUT8(vga_io, vga->FCTRL);
		VGA_MISC_OUT8(vga_io, vga->MISC);
#undef	VGA_RESTORE

	KRN_NOTICE("%s %s driver removed.\n",
		vga->chipset.vendor, vga->chipset.model);
}

#if 0
static void vga_text_chipset_adjust_timing(kgim_display_t *dpy, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_timing_command_t cmd,
	kgim_display_mode_t *kgim_mode)
{
	kgi_s_t foo;
	kgim_monitor_mode_t *crt_mode = KGIM_SUBSYSTEM_MODE(kgim_mode, monitor);

	KRN_ASSERT(img[0].out->dots.x == crt_mode->x.width); 
	KRN_ASSERT(img[0].out->dots.y == crt_mode->y.width);

	foo = img[0].out->dots.x / img[0].size.x;

	crt_mode->x.width	-= crt_mode->x.width % foo;
	crt_mode->x.blankstart	-= crt_mode->x.blankstart % foo;
	crt_mode->x.syncstart	-= crt_mode->x.syncstart % foo;
	crt_mode->x.syncend	-= crt_mode->x.syncend % foo;
	crt_mode->x.blankend	-= crt_mode->x.blankend % foo;

	if (crt_mode->x.total % foo) {

		crt_mode->x.total += foo - (crt_mode->x.total % foo);
	}

	if (((crt_mode->x.syncend / foo) & 0x3F) ==
		((crt_mode->x.syncstart / foo) & 0x3F)) {

		crt_mode->x.syncend -= foo;
	}

	if (((crt_mode->x.blankend / foo) & 0x7F) ==
		((crt_mode->x.blankstart / foo) & 0x7F)) {

		crt_mode->x.blankend -= foo;
	}

	if ((crt_mode->y.syncend & 0x0F) == (crt_mode->y.syncstart & 0x0F)) {

		crt_mode->y.syncend--;
	}

	if ((crt_mode->y.blankend & 0xFF) == (crt_mode->y.blankstart & 0xFF)) {

		crt_mode->y.blankend--;
	}
}
#endif


/* ----	text16 context operations ----------------------------------------------
**
**	We use the meta_mode and meta_object fields of kgic_mode_text16context
**	to store a reference to our kgim_display_t and kgim_display_mode_t.
*/
#define	VGA_CHIPSET_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, chipset)

#define	VGA_CHIPSET_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, chipset)

static void vga_text16_hc_show(kgic_mode_text16context_t *ctx,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_io_t    *vga_io   = VGA_CHIPSET_IO(ctx);
	vga_text_chipset_mode_t  *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u_t pos = vga_mode->orig_offs  +  x  +  y * ctx->virt.x;

	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	if (vga_mode->fb_size <= pos) {

		pos -= vga_mode->fb_size;
	}
	KRN_ASSERT(pos < vga_mode->fb_size);

	VGA_CRT_OUT8(vga_io, pos,      VGA_CRT_CURSORADDR_L);
	VGA_CRT_OUT8(vga_io, pos >> 8, VGA_CRT_CURSORADDR_H);
}

static void vga_text16_hc_hide(kgic_mode_text16context_t *ctx)
{
	vga_text_chipset_io_t *vga_io = VGA_CHIPSET_IO(ctx);

	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_H);
	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_L);
}

#define	vga_text16_hc_undo	NULL


/*	software cursor
*/
static void vga_text16_sc_show(kgic_mode_text16context_t *ctx,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	kgi_u32_t new = vga_mode->orig_offs + x + y * ctx->virt.x, old;

	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	if (vga_mode->fb_size < new) {

		new -= vga_mode->fb_size;
	}
	KRN_ASSERT(new < vga_mode->fb_size);

	if (vga_mode->cur.old & 0x8000) {

		mem_out16(vga_mode->cur.old >> 16, 
			(mem_vaddr_t) (fb + (vga_mode->cur.old & 0x7FFF)));
	}
	fb += new;
	old = mem_in16((mem_vaddr_t) fb);
	mem_out16((old & vga_mode->cur.and) ^ vga_mode->cur.xor, (mem_vaddr_t)fb);
	old <<= 16;
	if ((vga_mode->ptr.old & 0x8000) && 
		((vga_mode->ptr.old & 0x7FFF) == new)) {

		old = vga_mode->ptr.old & 0xFFFF0000;
	}
	vga_mode->cur.old = old | 0x8000;
}

static void vga_text16_sc_hide(kgic_mode_text16context_t *ctx)
{
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;

	if (vga_mode->cur.old & 0x8000) {

		mem_out16(vga_mode->cur.old >> 16,
			(mem_vaddr_t) (fb + (vga_mode->cur.old & 0x7FFF)));
	}
	vga_mode->cur.old = 0;
}

#define vga_text16_sc_undo	vga_text16_sc_hide


/*	software pointer
*/
static void vga_text16_sp_show(kgic_mode_text16context_t *ctx,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	kgi_u32_t new, old;

	x /= ctx->cell.x;
	y /= ctx->cell.y;
	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	new = vga_mode->orig_offs + x + y * ctx->virt.x;
	if (vga_mode->fb_size <= new) {

		new -= vga_mode->fb_size;
	}
	KRN_ASSERT(new < vga_mode->fb_size);

	if (vga_mode->ptr.old & 0x8000) {

		mem_out16(vga_mode->ptr.old >> 16,
			(mem_vaddr_t) (fb + (vga_mode->ptr.old & 0x7FFF)));
	}
	fb += new;
	old = mem_in16((mem_vaddr_t) fb);
	mem_out16((old & vga_mode->ptr.and) ^ vga_mode->ptr.xor,
		(mem_vaddr_t) fb);
	old <<= 16;
	if ((vga_mode->cur.old & 0x8000) && 
		((vga_mode->cur.old & 0x7FFF) == new)) {

		old = vga_mode->cur.old & 0xFFFF0000;
	}
	vga_mode->ptr.old = old | 0x8000;
}

static void vga_text16_sp_hide(kgic_mode_text16context_t *ctx)
{
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;

	if (vga_mode->ptr.old & 0x8000) {

		mem_out16(vga_mode->ptr.old >> 16,
			(mem_vaddr_t) (fb + (vga_mode->ptr.old & 0x7FFF)));
	}
	vga_mode->cur.old = 0;
}

#define	vga_text16_sp_undo	vga_text16_sp_hide

/*	depending on the ((vga_text_chipset_mode_t *) vga_mode)->flags setting,
**	we use either the hard- or software cursor marker.
*/
#define	vga_text16_cursor_show(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text16_hc_show : vga_text16_sc_show)
#define	vga_text16_cursor_hide(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text16_hc_hide : vga_text16_sc_hide)
#define	vga_text16_cursor_undo(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text16_hc_undo : vga_text16_sc_undo)

#define	vga_text16_pointer_show	vga_text16_sp_show
#define	vga_text16_pointer_hide	vga_text16_sp_hide
#define	vga_text16_pointer_undo	vga_text16_sp_undo

/*	text rendering
*/
static void vga_text16_put_text16(kgic_mode_text16context_t *ctx,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	mem_put16((mem_vaddr_t) (fb + offset), text, count);
}

/*	Texture look up table handling
*/
static void vga_text16_set_tlut(kgic_mode_text16context_t *ctx,
	kgi_u_t table, kgi_u_t index, kgi_u_t slots, void *tdata)
{
	vga_text_chipset_io_t   *vga_io   = VGA_CHIPSET_IO(ctx);
	vga_text_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u8_t *data = tdata;
	kgi_u8_t *fbuf = (kgi_u8_t *) vga_mode->fb.win.virt;
	kgi_u8_t SR2, SR4, GR4, GR5, GR6, CR3A;

	fbuf += (table * 0x2000) + (index << 5);
	data += ctx->font.y * index;

	/*	prepare hardware to access font plane
	*/

	/*	save register state
	*/
	SR2  = VGA_SEQ_IN8(vga_io, 0x02);
	SR4  = VGA_SEQ_IN8(vga_io, 0x04);
	GR4  = VGA_GRC_IN8(vga_io, 0x04);
	GR5  = VGA_GRC_IN8(vga_io, 0x05);
	GR6  = VGA_GRC_IN8(vga_io, 0x06);
	CR3A = VGA_CRT_IN8(vga_io, 0x3A);

	VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02); /* write to plane 2  */
	VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);
	VGA_GRC_OUT8(vga_io, 2, 0x04); /* read from plane 2 */
	VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
	VGA_GRC_OUT8(vga_io, (GR6 & VGA_GR6_MAP_MASK) | VGA_GR6_GRAPHMODE,0x06);

	/* NOTE	no KRN_DEBUG/NOTICE/ASSERT allowed in here!
	*/
	if (tdata) {

		/*	set font if data is valid
		*/
		register kgi_s_t j, cnt = 32 - ctx->font.y;

		while (slots--) {

			for (j = ctx->font.y; j--; ) {

				mem_out8(*(data++), fbuf++);
			}
			for (j = cnt; j--; ) {

				mem_out8(0, fbuf++);
			}
		}

	} else {

		register kgi_u32_t *h = (void *) fbuf;

		/*	clear font
		*/
		while (slots--) {

			mem_out32(0, (mem_vaddr_t) (h + 0));
			mem_out32(0, (mem_vaddr_t) (h + 1));
			mem_out32(0, (mem_vaddr_t) (h + 2));
			mem_out32(0, (mem_vaddr_t) (h + 3));
			mem_out32(0, (mem_vaddr_t) (h + 4));
			mem_out32(0, (mem_vaddr_t) (h + 5));
			mem_out32(0, (mem_vaddr_t) (h + 6));
			mem_out32(0, (mem_vaddr_t) (h + 7));
			h += 8;
		}
	}

	/*	restore hardware state
	*/
/* !!!	This should go somewhere else!

	VGA_CRT_OUT8(vga_io,
		(VGA_CRT_IN8(vga_io, 0x0A) & ~VGA_CR0A_CURSOR_START_MASK) |
		((font->base + 1) & VGA_CR0A_CURSOR_START_MASK), 0x0A);
	VGA_CRT_OUT8(vga_io,
		(VGA_CRT_IN8(vga_io, 0x0B) & ~VGA_CR0B_CURSOR_END_MASK) |
		((font->base + 2) & VGA_CR0B_CURSOR_END_MASK), 0x0B);
*/
	VGA_SEQ_OUT8(vga_io, SR2,  0x02);
	VGA_SEQ_OUT8(vga_io, SR4,  0x04);
	VGA_GRC_OUT8(vga_io, GR4,  0x04);
	VGA_GRC_OUT8(vga_io, GR5,  0x05);
	VGA_GRC_OUT8(vga_io, GR6,  0x06);
	VGA_CRT_OUT8(vga_io, CR3A, 0x3A);
}

#undef	VGA_CHIPSET_MODE
#undef	VGA_CHIPSET_IO
/*
**
** ----	end of text16 context functions ----------------------------------------
*/

/* ----	begin of mode_check() functions ---------------------------------------
**
*/
static kgi_error_t vga_text16_command(kgic_mode_context_t *ctx, kgi_u_t cmd,
	void *in_buffer, void **out_buffer, kgi_size_t *out_size)
{
	kgim_display_mode_t *dpy_mode = ctx->dev_mode;
	vga_text_chipset_mode_t *vga_mode = KGIM_SUBSYSTEM_MODE(dpy_mode, chipset);

	switch (cmd) {

	case KGIC_MODE_TEXT16CONTEXT:
		{
			kgic_mode_text16context_t *out = *out_buffer;
			kgic_mode_text16context_request_t *in = in_buffer;

			if (in->image) {

				KRN_DEBUG(1, "invalid image %i", in->image);
				return -KGI_ERRNO(DRIVER, INVAL);
			}

			out->revision    = KGIC_MODE_TEXT16CONTEXT_REVISION;
			out->meta_object = ctx->dpy;
			out->meta_mode	 = ctx->dev_mode;
			out->size.x      = ctx->img->size.x;
			out->size.y      = ctx->img->size.y;
			out->virt.x      = ctx->img->virt.x;
			out->virt.y      = ctx->img->virt.y;
			out->cell.x      = 9;
			out->cell.y      = 16;
			out->font.x      = 8;
			out->font.y      = 16;
			out->CursorShow  = vga_text16_cursor_show(vga_mode);
			out->CursorHide  = vga_text16_cursor_hide(vga_mode);
			out->CursorUndo  = vga_text16_cursor_undo(vga_mode);
			out->PointerShow = vga_text16_pointer_show;
			out->PointerHide = vga_text16_pointer_hide;
			out->PointerUndo = vga_text16_pointer_undo;
			out->PutText16   = vga_text16_put_text16;
			out->SetCLUT     = NULL;
			out->SetTLUT     = vga_text16_set_tlut;
			return KGI_EOK;
		}

	default:
		KRN_DEBUG(1, "unknown/unsupported mode command %.8x", cmd);
		return -KGI_ERRNO(DRIVER, INVAL);
	}
}

/*	This is the SetOffset() handler for the exported memory mapped
**	I/O region. We export a 64KB region, the first 32K being a linear
**	view of the text buffer and the second 32K being a linear view
**	of the font buffer.
*/
static void vga_text16_fb_set_offset(kgi_mmio_region_t *r, kgi_size_t offset) 
{
	vga_text_chipset_io_t *vga_io = r->meta_io;

	/*	first half is text16 frame buffer, second half is font buffer
	*/
	if (offset < 32 KB) {

		/*	setup for text buffer access
		*/
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE0 | VGA_SR02_PLANE1, 0x02);
		VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS, 0x04);

		VGA_GRC_OUT8(vga_io, 0, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_ODD_EVEN, 0x05);
		VGA_GRC_OUT8(vga_io, VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE,0x06);

	} else {

		/*	setup for font buffer access
		*/
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02);
		VGA_SEQ_OUT8(vga_io, 
			VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);

		VGA_GRC_OUT8(vga_io, 2, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
		VGA_GRC_OUT8(vga_io,
			VGA_GR6_MAP_B8_32 | VGA_GR6_GRAPHMODE, 0x06);
	}
}

static const kgi_u8_t vga_default_SEQ[VGA_SEQ_REGS] =
{
	0x03, 0x00, 0x0F, 0x20, 0x02
};

static const kgi_u8_t vga_default_CRT[VGA_CRT_REGS] =
{
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x10,		/* 0x00	*/
	0x00, 0x40, 0x00, 0x1F, 0x00, 0x00, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0xE3,		/* 0x1x	*/
	0xFF
};

static const kgi_u8_t vga_default_GRC[VGA_GRC_REGS] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* GRC	*/
	0xFF
};

static const kgi_u8_t vga_default_ATC[VGA_ATC_REGS] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,		/* 0x00	*/
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x84, 0x00, 0x0F, 0x00, 0x00				/* 0x1x	*/
};

static const kgi_u8_t vga_text_chipset_448[] = { 4,4,8,0 };

kgi_error_t vga_text_chipset_mode_check(vga_text_chipset_t *vga,
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img[0].out;
	const kgim_monitor_mode_t *crt_mode = vga_mode->kgim.crt;
	kgi_ucoord_t text, maxsize;
	kgi_u_t foo, mul, bpp;
	kgi_u_t DCLKdiv;

	if (images != 1) {

		KRN_DEBUG(1, "%i images are not supported.", images);
		return -KGI_ERRNO(CHIPSET,NOSUP);
	}

	/*	for unsupported image flags, bail out
	*/
	if (img[0].flags & (KGI_IF_VIRTUAL | KGI_IF_VISIBLE | KGI_IF_TILE_X |
		KGI_IF_STEREO)) {

		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	common attributes are not possible
	*/
	if (img[0].cam) {

		KRN_DEBUG(1, "common attributes %.8x not supported",
			img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	frame attributes must be Index4 Foreground4 Texture8
	*/
	if ((img[0].fam != KGI_AM_TEXT) || 
		kgim_strcmp(img[0].bpfa, vga_text_chipset_448)) {

		KRN_DEBUG(1, "pixel attributes not supported");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	KRN_ASSERT(img[0].frames);
	bpp = img[0].frames * 16;

	if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

		img[0].size.x = 80;
		img[0].size.y = 25;
		text.x = 9;
		text.y = 16;

	} else {

		text.x = dpm->dots.x / img[0].size.x;
		text.y = dpm->dots.y / img[0].size.y;
	}

	if ((dpm->flags & KGI_DPF_TP_MASK) == KGI_DPF_TP_LRTB_I1) {

		KRN_DEBUG(1, "interlaced modes not supported.");
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);
		KRN_ASSERT(text.x && text.y);

		maxsize.x = vga->chipset.maxdots.x / text.x;
		maxsize.y = vga->chipset.maxdots.y / text.y;
		KRN_DEBUG(2, "text grid %ix%i, maxsize %ix%i, maxdots %ix%i",
			text.x, text.y, maxsize.x, maxsize.y,
			vga->chipset.maxdots.x, vga->chipset.maxdots.y);

		/*	if virt.x and virt.y are zero, default to (size.x, max)
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
		}
		if (0 == img[0].virt.x) {

			img[0].virt.x =
				vga->chipset.memory / (bpp * img[0].virt.y);

			if (img[0].virt.x > maxsize.x) {

				img[0].virt.x = maxsize.x;
			}
		}
		if (0 == img[0].virt.y) {

			img[0].virt.y =
				vga->chipset.memory / (bpp * img[0].virt.x);

			if (img[0].virt.y > maxsize.y) {

				img[0].virt.y = maxsize.y;
			}
		}

		if ((img[0].size.x > maxsize.x) || 
			(img[0].virt.x > maxsize.x)) {

			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many.",
				img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.y > maxsize.y) ||
			(img[0].virt.y > maxsize.y)) {

			KRN_DEBUG(1, "%i (%i) vertical pixels are too many.",
				img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 8*vga->textmemory) {

			KRN_DEBUG(1, "no text memory (%i byte) for %ix%i");
			return -KGI_ERRNO(CHIPSET, NOMEM);
		}

		if (((text.x != 8) && (text.x != 9)) || 
			(text.y < 1) || (text.y > 32)) {

			KRN_DEBUG(1, "invalid text font size (%ix%i)",
				text.x, text.y);
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}

		dpm->dots.x = img[0].size.x * text.x;
		dpm->dots.y = img[0].size.y * text.y;
		KRN_DEBUG(2, "dotport mode with %ix%i dots", 
			dpm->dots.x, dpm->dots.y);
		return KGI_EOK;

	case KGI_TC_LOWER:

		if (dpm->dclk < vga->chipset.dclk.min) {

			KRN_DEBUG(1, "%i Hz DCLK is too low.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk > vga->chipset.dclk.max)
			? vga->chipset.dclk.max : dpm->dclk;

/* !!!		vga_text_chipset_adjust_timing(img, mode); */
		return KGI_EOK;

	case KGI_TC_RAISE:

		if (dpm->dclk > vga->chipset.dclk.max) {

			KRN_DEBUG(1, "%i Hz DCLK is too high.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk < vga->chipset.dclk.min)
			? vga->chipset.dclk.min : dpm->dclk;

/*		vga_text_chipset_adjust_timing(img, mode); */
		return KGI_EOK;

	case KGI_TC_CHECK:

		if ((KGI_AM_TEXT != dpm->dam) || 
			kgim_strcmp(dpm->bpda, vga_text_chipset_448)) {

			KRN_DEBUG(1, "dot port attributes %.8x not supported",
				dpm->dam);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((dpm->dclk > vga->chipset.dclk.max) ||
			(dpm->dclk < vga->chipset.dclk.min)) {

			KRN_DEBUG(1, "DCLK %i Hz is out of limits.",
				dpm->dclk);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (((8 != text.x) && (9 != text.x)) || 
			((text.y < 1) || (32 < text.y))) {

			KRN_DEBUG(1, "font size %ix%i not supported",
				text.x, text.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) < vga->textmemory) {

			KRN_DEBUG(1, "not enough text memory for %ix%i virt.",
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

#		define TM(X) \
			((vga->chipset.maxdots.x < crt_mode->x.X) || \
			 (vga->chipset.maxdots.y < crt_mode->y.X))

			if (TM(width) || TM(blankstart) || TM(syncstart) ||
				TM(syncend) || TM(blankend) || TM(total)) {

				KRN_DEBUG(1, "timing check failed");
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
#		undef TM

/*		vga_text_chipset_adjust_timing(img, mode); */
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/*	Now everthing is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	KRN_DEBUG(2, "setting up for mode [%.8x@%ix%i] %ix%i (%ix%i)",
		dpm->dam, dpm->dots.x, dpm->dots.y,
		img[0].size.x, img[0].size.y,
		img[0].virt.x, img[0].virt.y);

	mul = crt_mode->in.rclk.mul;

	/*	set default values
	*/
	vga_mode->kgim.Command   = vga_text16_command;

	vga_mode->MISC		= 0x23;
	kgim_memcpy(vga_mode->SEQ, vga_default_SEQ, sizeof(vga_mode->SEQ));
	kgim_memcpy(vga_mode->CRT, vga_default_CRT, sizeof(vga_mode->CRT));
	kgim_memcpy(vga_mode->GRC, vga_default_GRC, sizeof(vga_mode->GRC));
	kgim_memcpy(vga_mode->ATC, vga_default_ATC, sizeof(vga_mode->ATC));
	vga_mode->flags		= VGA_CMF_HW_CURSOR;
	vga_mode->cur.and	= 0x00FF;
	vga_mode->cur.xor	= 0x7F00;
	vga_mode->ptr.and	= 0x00FF;
	vga_mode->ptr.xor	= 0x6F00;
	vga_mode->orig_dot_x	= 0;
	vga_mode->orig_dot_y	= 0;
	vga_mode->orig_offs	= 0;
	vga_mode->fb_size	= img[0].virt.x * img[0].virt.y;

	/*	frame buffer region
	*/
	vga_mode->fb.meta       = vga;
	vga_mode->fb.meta_io    = vga_io;
	vga_mode->fb.type	= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->fb.prot	= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->fb.name	= "text16 buffer";
	vga_mode->fb.access	= 8 + 16 + 32 + 64;
	vga_mode->fb.align	= 0;
	vga_mode->fb.size	= 64 KB;
	vga_mode->fb.win.size	= 32 KB;
	vga_mode->fb.win.bus	= vga_io->aperture.base_bus;
	vga_mode->fb.win.phys	= vga_io->aperture.base_phys;
	vga_mode->fb.win.virt	= vga_io->aperture.base_virt;
	vga_mode->fb.SetOffset	= vga_text16_fb_set_offset;

	/*	Setup bits per pixel and logical screen width
	*/
	foo = img[0].virt.x / 2;
	vga_mode->CRT[0x13] |= foo & VGA_CR13_LOGICALWIDTH_MASK;
	vga_mode->CRT[0x14] |= VGA_CR14_UNDERLINE_MASK;
	vga_mode->CRT[0x17] = VGA_CR17_CGA_ADDR_WRAP | VGA_CR17_ENABLE_SYNC |
		VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING;

	vga_mode->SEQ[0x02] = VGA_SR02_PLANE0 | VGA_SR02_PLANE1;

	vga_mode->GRC[0x05] |= VGA_GR5_ODD_EVEN;
	vga_mode->GRC[0x06] |= VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE;

	switch ((DCLKdiv = img[0].out->dots.x / img[0].size.x)) {

	case 8:
		vga_mode->SEQ[0x01] |= VGA_SR01_8DOT_CHCLK;
		break;

	case 9:
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	vga_mode->CRT[0x09] |= img[0].out->dots.y/img[0].size.y - 1;
	vga_mode->ATC[0x13] = (DCLKdiv == 9) ? 0x08 : 0x00;

	/*	setup horizontal timing
	*/
	foo = (crt_mode->x.width * mul / DCLKdiv) - 1;
	vga_mode->CRT[0x01] |= foo & 0xFF;

	foo = (crt_mode->x.blankstart * mul / DCLKdiv);
	vga_mode->CRT[0x02] |= foo & 0xFF;

	foo = (crt_mode->x.syncstart * mul / DCLKdiv);
	vga_mode->CRT[0x04] |= foo & 0xFF;

	foo = (crt_mode->x.syncend * mul / DCLKdiv);
	vga_mode->CRT[0x05] |= foo & VGA_CR05_HSE_MASK;

	foo = (crt_mode->x.blankend * mul / DCLKdiv) - 1;
	vga_mode->CRT[0x03] |= foo & VGA_CR03_HBE_MASK;
	vga_mode->CRT[0x05] |= (foo & 0x20) ? VGA_CR05_HBLANKEND_B5 : 0;

	foo = (crt_mode->x.total * mul / DCLKdiv) - 5;
	vga_mode->CRT[0x00] |= foo & 0xFF;

	/*	setup vertical timing
	*/
	foo = crt_mode->y.width - 1;
	vga_mode->CRT[0x12] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VDISPLAYEND_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VDISPLAYEND_B9 : 0;

	foo = crt_mode->y.blankstart - 1;
	vga_mode->CRT[0x15] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VBLANKSTART_B8 : 0;
	vga_mode->CRT[0x09] |= (foo & 0x200) ? VGA_CR09_VBLANKSTART_B9 : 0;

	foo = crt_mode->y.syncstart;
	vga_mode->CRT[0x10] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VSYNCSTART_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VSYNCSTART_B9 : 0;

	foo = crt_mode->y.syncend;
	vga_mode->CRT[0x11] |= foo & VGA_CR11_VSYNCEND_MASK;

	foo = crt_mode->y.blankend - 1;
	vga_mode->CRT[0x16] |= foo & 0xFF;

	foo = crt_mode->y.total - 2;
	vga_mode->CRT[0x06] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VTOTAL_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VTOTAL_B9 : 0;

	vga_mode->MISC |= (crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC;
	vga_mode->MISC |= (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC;

	return KGI_EOK;
}

kgi_resource_t *vga_text_chipset_mode_resource(vga_text_chipset_t *vga, 
	vga_text_chipset_mode_t *vga_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	return (0 == index) ? (kgi_resource_t *) &vga_mode->fb : NULL;
}

void vga_text_chipset_mode_prepare(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	VGA_SEQ_OUT8(vga_io,
		VGA_SEQ_IN8(vga_io, 0x01) | VGA_SR01_DISPLAY_OFF, 0x01);
}

void vga_text_chipset_mode_enter(vga_text_chipset_t *vga,
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t i;

	VGA_MISC_OUT8(vga_io,
		(VGA_MISC_IN8(vga_io) & VGA_MISC_CLOCK_MASK) | vga_mode->MISC);
	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);

#define	VGA_SET(subsys)							\
		for (i = 0; i < VGA_##subsys##_REGS; i++) {		\
									\
			VGA_##subsys##_OUT8(vga_io, vga_mode->subsys[i], i);\
		}

		VGA_SET(SEQ);
		VGA_CRT_OUT8(vga_io,
			VGA_CRT_IN8(vga_io, 0x11) & ~VGA_CR11_LOCKTIMING, 0x11);
		VGA_SET(CRT);
		VGA_SET(GRC);
		VGA_SET(ATC);
#undef	VGA_SET
	*((kgi_u32_t *) vga_mode->fb.win.virt) = 0x12345678;
}
