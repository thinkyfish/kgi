/* ----------------------------------------------------------------------------
**	IBM VGA chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:21  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#define	__IBM_VGA
#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-meta.h"
#include "chipset/IBM/VGA-bind.h"


#ifdef DEBUG_LEVEL
/*	Print verbose chipset configuration for debugging purposes
*/
static inline void vga_chipset_examine(vga_chipset_t *vga, kgi_u32_t flags)
{
	kgi_u32_t mclk, memsize;
	kgi_u32_t foo, i;
	
	KRN_DEBUG(2, "vga_chipset_examine()");
	
	KRN_DEBUG(2, "");
	
	KRN_DEBUG(2, "SEQ Registers:");
	KRN_DEBUG(2, "--------------");
	
	for (i = 0; i < VGA_SEQ_REGS; i++)
	{
		KRN_DEBUG(2, "SEQ%.2x = %.2x", i, vga->SEQ[i]);
	}
}
#endif	/* #if (DEBUG_LEVEL) */

static inline void vga_chipset_sync(vga_chipset_io_t *vga_io)
{
	KRN_DEBUG(2, "vga_chipset_sync()");
	
	return;
}


void vga_chipset_mode_leave(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "vga_chipset_mode_leave()");
	
	vga_chipset_sync(vga_io);

/*	vga->mode = NULL;
*/
}

kgi_error_t vga_chipset_init(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	kgi_u_t i;
	
	KRN_DEBUG(2, "vga_chipset_init()");

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	kgim_memset(vga, 0, sizeof(*vga));
	
	kgim_strcpy(vga->chipset.vendor, "IBM");
	kgim_strcpy(vga->chipset.model, "VGA");
	
	vga->chipset.revision	= KGIM_CHIPSET_REVISION;
	vga->chipset.flags	= KGIM_CF_NORMAL;
	vga->chipset.mode_size	= sizeof(vga_chipset_mode_t);

	vga->chipset.maxdots.x	= 920;
	vga->chipset.maxdots.y	= 550;
	vga->chipset.memory	= 256 KB;
	
	vga->chipset.dclk.min	= 12000000;
	vga->chipset.dclk.max	= 32000000;
	
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
//     	VGA_SAVE(ATC); // FIXME: ATC reads or writes freeze my display [jtaylor]

#undef	VGA_SAVE

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		vga->chipset.vendor, vga->chipset.model);
	
	return KGI_EOK;
}

void vga_chipset_done(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_chipset_done()");

#define	VGA_RESTORE(subsys)					\
	for (i = 0; i < VGA_##subsys##_REGS; i++) {		\
								\
		VGA_##subsys##_OUT8(vga_io, vga->subsys[i], i);	\
	}

	VGA_RESTORE(SEQ);
	VGA_CRT_OUT8(vga_io,
		VGA_CRT_IN8(vga_io, 0x11) & ~VGA_CR11_LOCKTIMING, 0x11);
	VGA_RESTORE(CRT);
	VGA_RESTORE(GRC);
/*	VGA_RESTORE(ATC);
*/
	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);
	VGA_MISC_OUT8(vga_io, vga->MISC);
	
#undef	VGA_RESTORE

	KRN_NOTICE("%s %s driver removed.\n", 
		vga->chipset.vendor, vga->chipset.model);
}

static void vga_chipset_adjust_timing(vga_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img)
{
	kgi_s_t foo;
	const kgim_monitor_mode_t *crt_mode = vga_mode->kgim.crt;
	kgi_dot_port_mode_t *dpm = img[0].out;

	KRN_DEBUG(2, "vga_chipset_adjust_timing()");
		   
	if (img[0].fam == KGI_AM_TEXT) {

		foo = dpm->dots.x / img[0].size.x;

	} else {

		/* FIXME: Don't hardwire 8bpp here */
		foo = 8;
	}

	KRN_ASSERT(dpm->dots.x == crt_mode->x.width); 
	KRN_ASSERT(dpm->dots.y == crt_mode->y.width);

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
	vga_chipset_io_t    *vga_io   = VGA_CHIPSET_IO(ctx);
	vga_chipset_mode_t  *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u_t pos = vga_mode->orig_offs  +  x  +  y * ctx->virt.x;
	
	KRN_DEBUG(2,"vga_text16_hc_show()");

	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	if (vga_mode->text16fb_size <= pos) {

		pos -= vga_mode->text16fb_size;
	}
	
	KRN_ASSERT(pos < vga_mode->text16fb_size);

	VGA_CRT_OUT8(vga_io, pos,      VGA_CRT_CURSORADDR_L);
	VGA_CRT_OUT8(vga_io, pos >> 8, VGA_CRT_CURSORADDR_H);
}

static void vga_text16_hc_hide(kgic_mode_text16context_t *ctx)
{
	vga_chipset_io_t *vga_io = VGA_CHIPSET_IO(ctx);

	KRN_DEBUG(2,"vga_text16_hc_hide()");

	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_H);
	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_L);
}


#define	vga_text16_hc_undo	NULL


/* Software cursor */
static void vga_text16_sc_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->text16fb.win.virt;
	kgi_u32_t new = vga_mode->orig_offs + x + y * ctx->virt.x;
	kgi_u32_t old;

	KRN_DEBUG(2, "vga_text16_sc_show()");

	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	if (vga_mode->text16fb_size < new) {

		new -= vga_mode->text16fb_size;
	}
	
	KRN_ASSERT(new < vga_mode->text16fb_size);

	if (vga_mode->cur.old & 0x8000) {

		mem_out16(vga_mode->cur.old >> 16, 
			(mem_vaddr_t) (fb + (vga_mode->cur.old & 0x7FFF)));
	}
	
	fb += new;
	old = mem_in16((mem_vaddr_t) fb);
	mem_out16((old & vga_mode->cur.and) ^ vga_mode->cur.xor,
		(mem_vaddr_t)fb);
	old <<= 16;
	
	if ((vga_mode->ptr.old & 0x8000) && 
		((vga_mode->ptr.old & 0x7FFF) == new)) {

		old = vga_mode->ptr.old & 0xFFFF0000;
	}
	
	vga_mode->cur.old = old | 0x8000;
}

static void vga_text16_sc_hide(kgic_mode_text16context_t *ctx)
{
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->text16fb.win.virt;

	KRN_DEBUG(2, "vga_text16_hc_hide()");

	if (vga_mode->cur.old & 0x8000) {

		mem_out16(vga_mode->cur.old >> 16, 
			(mem_vaddr_t) (fb + (vga_mode->cur.old & 0x7FFF)));
	}
	
	vga_mode->cur.old = 0;
}

#define vga_text16_sc_undo	vga_text16_sc_hide


/*	Software pointer
*/
static void vga_text16_sp_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->text16fb.win.virt;
	kgi_u32_t new, old;

	KRN_DEBUG(2, "vga_text16_sp_show()");

	x /= ctx->cell.x;
	y /= ctx->cell.y;
	
	KRN_ASSERT((x < ctx->size.x) && (y < ctx->size.y));

	new = vga_mode->orig_offs + x + y * ctx->virt.x;
	
	if (vga_mode->text16fb_size <= new) {

		new -= vga_mode->text16fb_size;
	}
	
	KRN_ASSERT(new < vga_mode->text16fb_size);

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
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->text16fb.win.virt;

	KRN_DEBUG(2, "vga_text16_sp_hide()");

	if (vga_mode->ptr.old & 0x8000) {

		mem_out16(vga_mode->ptr.old >> 16, 
			(mem_vaddr_t) (fb + (vga_mode->ptr.old & 0x7FFF)));
	}
	
	vga_mode->cur.old = 0;
}

#define	vga_text16_sp_undo	vga_text16_sp_hide

/*	depending on the ((vga_chipset_mode_t *) vga_mode)->flags setting,
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

/*	Text rendering
*/
static void vga_text16_put_text16(kgic_mode_text16context_t *ctx,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->text16fb.win.virt;

	KRN_DEBUG(2, "vga_text16_put_text16()");

	mem_put16((mem_vaddr_t) (fb + offset), text, count);
}

/*	Texture look up table handling
*/
static void vga_text16_set_tlut(kgic_mode_text16context_t *ctx,
	kgi_u_t table, kgi_u_t index, kgi_u_t slots, void *tdata)
{
	vga_chipset_io_t   *vga_io   = VGA_CHIPSET_IO(ctx);
	vga_chipset_mode_t *vga_mode = VGA_CHIPSET_MODE(ctx);

	kgi_u8_t *data = tdata;
	kgi_u8_t *fbuf = (kgi_u8_t *) vga_mode->text16fb.win.virt;
	kgi_u8_t SR2, SR4, GR4, GR5, GR6, CR3A;

	KRN_DEBUG(2, "vga_text16_set_tlut()");

	fbuf += (table * 0x2000) + (index << 5);
	data += ctx->font.y * index;

	/*	Prepare hardware to access font plane
	*/

	/*	Save register state
	*/
	SR2  = VGA_SEQ_IN8(vga_io, 0x02);
	SR4  = VGA_SEQ_IN8(vga_io, 0x04);
	GR4  = VGA_GRC_IN8(vga_io, 0x04);
	GR5  = VGA_GRC_IN8(vga_io, 0x05);
	GR6  = VGA_GRC_IN8(vga_io, 0x06);
	CR3A = VGA_CRT_IN8(vga_io, 0x3A);

	VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02); /* Write to plane 2  */
	VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);
	VGA_GRC_OUT8(vga_io, 2, 0x04); /* Read from plane 2 */
	VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
	VGA_GRC_OUT8(vga_io, (GR6 & VGA_GR6_MAP_MASK) | VGA_GR6_GRAPHMODE,0x06);
	
	/*	NOTE	no KRN_DEBUG/NOTICE/ASSERT allowed in here!
	*/
	/*	FIXME:	We should use a critical section or spinlock then 
	**		[taylor_j]
	*/
	if (tdata) {

		/*	Set font if data is valid
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

		/*	Clear font
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

	/*	Restore hardware state
	*/
	
/* !!!	This should go somewhere else!
** 
**	Perhaps we could just define a set of registers which are flushed only
**	at every VSYNC, and make these registers part of that set? [taylor_j]
**
**	The reason this should go somewhere else is that this sets cursor
**	properties, which are a-priori not related to the font. [seeger_s]
**
**	VGA_CRT_OUT8(vga_io,
**		(VGA_CRT_IN8(vga_io, 0x0A) & ~VGA_CR0A_CURSOR_START_MASK) |
**		((font->base + 1) & VGA_CR0A_CURSOR_START_MASK), 0x0A);
**	VGA_CRT_OUT8(vga_io,
**		(VGA_CRT_IN8(vga_io, 0x0B) & ~VGA_CR0B_CURSOR_END_MASK) |
**		((font->base + 2) & VGA_CR0B_CURSOR_END_MASK), 0x0B);
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

/*  ---- end of text16 context functions ------------------------------------ */

/* ----	begin of mode_check() functions ------------------------------------- */

/* FIXME	How to properly handle this from vga_chipset_command?
*/
static kgi_error_t vga_text16_command(kgic_mode_context_t *ctx, 
	kgi_u_t cmd, void *in_buffer, void **out_buffer, kgi_size_t *out_size)
{
	kgim_display_mode_t *dpy_mode = ctx->dev_mode;
	vga_chipset_mode_t *vga_mode = KGIM_SUBSYSTEM_MODE(dpy_mode, chipset);
	
	KRN_DEBUG(2, "vga_text16_command()");

	switch (cmd) {

	case KGIC_MODE_TEXT16CONTEXT:
		KRN_DEBUG(2,"KGIC_MODE_TEXT16CONTEXT:");
		{
			kgic_mode_text16context_t *out = *out_buffer;
			kgic_mode_text16context_request_t *in = in_buffer;

			if (in->image) 
			{
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
#warning - IIRC you must set these! -- seeger_s			
/*			out->cell.x      = 9;
**			out->cell.y      = 16;
**			out->font.x      = 8;
*/			out->font.y      = 16;

			out->cell.x      = ctx->img->out->dots.x;
			out->cell.y      = ctx->img->out->dots.y;
			out->font.x      = ctx->img->out->dots.x;
			out->font.y      = ctx->img->out->dots.y;

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
**
** 	FIXME: I do not like the idea of directly exposing the font buffer.
** 	Since it is being used for the font-hack pointer, we cannot allow
** 	userspace to corrupt it.  We should buffer the font table descriptor
** 	that userspace passes to us, and flush it to hardware on the next
** 	VSYNC or something.  [taylor_j]
**
**	PROPOSED FIX: Drop the font hack cursor in favour of
**	userspace processes being allowed access to the font tables.
**	A simple block cursor as implemented above does have the very same
**	functionality as the font hack cursor. The VGA driver might
**	serve as reference driver for LOADS of others, so I refuse to
**	burden it with unneccessary functionality. [seeger_s]
*/
static void vga_text16_fb_set_offset(kgi_mmio_region_t *r, kgi_size_t offset) 
{
	vga_chipset_io_t *vga_io = r->meta_io;

	KRN_DEBUG(2, "vga_text16_fb_set_offset()");

	/* First half is text16 frame buffer, second half is font buffer */
	if (offset < 32 KB) 
	{
		/* Set up for text buffer access */
		KRN_DEBUG(2,"Setting up for text buffer access");
		
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE0 | VGA_SR02_PLANE1, 0x02);
		VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS, 0x04);

		VGA_GRC_OUT8(vga_io, 0, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_ODD_EVEN, 0x05);
		VGA_GRC_OUT8(vga_io, VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE,0x06);
	} else 
	{
		/* Set up for font buffer access */
		KRN_DEBUG(2,"Setting up for font buffer access");
		
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02);
		VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);

		VGA_GRC_OUT8(vga_io, 2, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
		VGA_GRC_OUT8(vga_io, VGA_GR6_MAP_B8_32 | VGA_GR6_GRAPHMODE, 0x06);
	}
}

/*	FIXME: Dumb arrays of default registers are _evil_.
**	It is a lot easier to see what is going on when
**	you assign each register a value procedurally
**	in the init and mode setup code, because you
**	can use the REGISTER_DEFINES which are a lot
**	easier to grasp instantly, unlike 0xb3 which
**	you have to look up in a reference or have
**	memorized.  It makes the code a little longer,
**	but a LOT easier to work with.  This stuff has
**	to go.  -- [taylor_j]
**	
**	So go ahead :-)) [seeger_s]
*/

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
	0x81, 0x00, 0x0F, 0x00, 0x00				/* 0x1x	*/
};

static const kgi_u8_t vga_chipset_448[] = { 4,4,8,0 };


/*	FIXME: Do all text16 checking in another function,
**	and call it from here when we get a text16 mode [taylor_j]
**
**	FIX: That's what the VGA-text meta language is for! Don't reinvent
**	the wheel, reuse the VGA text meta language or improve it if
**	neccessary. VGA-meta.c should only care about graphic modes and
**	fall back to VGA-text-meta.c wherever possible.	[seeger_s]
*/
kgi_error_t vga_chipset_mode_check(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_timing_command_t cmd, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img[0].out;
	const kgim_monitor_mode_t *crt_mode = vga_mode->kgim.crt;
	kgi_ucoord_t text, maxsize;
	kgi_u_t foo, mul, bpp, lclk;
	kgi_u_t DCLKdiv;
	kgi_u_t bpf, bpc, bpd, shift, pgm;
	
	KRN_DEBUG(2, "vga_chipset_mode_check()");
	
	KRN_DEBUG(2, "dpm->dots.[x,y] = [%.8x,%.8x]", dpm->dots.x, dpm->dots.y);
	
	text.x = text.y = 0;
	
	if (images != 1) {

		KRN_DEBUG(1, "%i images are not supported.", images);
		return -KGI_ERRNO(CHIPSET,NOSUP);
	}
	
	/*	For unsupported image flags, bail out
	*/
	if (img[0].flags & (KGI_IF_VIRTUAL | KGI_IF_VISIBLE | 
		KGI_IF_TILE_X | KGI_IF_STEREO)) {

		KRN_DEBUG(1, "Image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	KRN_ASSERT(img[0].frames);
	
	if (img[0].fam == KGI_AM_TEXT) {

		KRN_DEBUG(2, "Checking a textmode....");
		
		KRN_DEBUG(2, "maxsize.x = %i", maxsize.x);
		KRN_DEBUG(2, "maxsize.y = %i", maxsize.y);
		
		/* Common attributes are not possible */
		if (img[0].cam) {

			KRN_DEBUG(1, "Common attributes %.8x not supported",
				img[0].cam);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		/*	Frame attributes must be Index4 Foreground4 Texture8
		*/
		if (kgim_strcmp(img[0].bpfa, vga_chipset_448)) {

			KRN_DEBUG(1, "Pixel attributes not supported");
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		bpp = img[0].frames * 16;
		
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			KRN_DEBUG(2, "size.x or size.y == 0; "
				"defaulting to 80x25[9x16]");
			
			img[0].size.x = 80;
			img[0].size.y = 25;
			text.x = 9;
			text.y = 16;
			
		} else {

			KRN_DEBUG(2, "case a");
			KRN_DEBUG(2, "img[0].size = {%d,%d}", 
				img[0].size.x, img[0].size.y);
			KRN_DEBUG(2, "dpm->dots = {%d,%d}",
				dpm->dots.x, dpm->dots.y);
			KRN_DEBUG(2, "text.[x,y] = [%d,%d]", text.x, text.y);
			
			if ((text.x == 0) || (text.y == 0)) {

				KRN_DEBUG(2, "case z");
				text.x = 8;
				text.y = 16;
			}
			
			KRN_DEBUG(2, "dpm->dots = {%d,%d}",
				dpm->dots.x, dpm->dots.y);
			KRN_DEBUG(2, "text = {%d,%d}",
				text.x, text.y);
		}
		
		KRN_DEBUG(2, "text.x = %.3d, text.y = %.3d", text.x, text.y);

	} else {

		/*	Not a textmode
		*/
		
		bpf = kgim_attr_bits(img[0].bpfa);
		bpc = kgim_attr_bits(img[0].bpca);
		bpd = kgim_attr_bits(dpm->bpda);
		bpp = (bpc + bpf * img[0].frames);
		
		KRN_DEBUG(2, "bpf = %i, bpc = %i, bpd = %i, bpp = %i",
			bpf, bpc, bpd, bpp);
		
		/*	Check if common attributes are supported 
		*/
		switch (img[0].cam) {

		case 0: break;

		case KGI_AM_COLOR_INDEX:
			KRN_DEBUG(2, "KGI_AM_COLOR_INDEX");
		
			if (bpc != 8) {

				KRN_DEBUG(1, "Non-8bpp graphic modes "
					"not yet supported");
				return -KGI_ERRNO(CHIPSET, INVAL);

			} else {

				vga_mode->flags |= VGA_CMF_8BPPMODE;
				dpm->lclk.mul = 1;

#warning These are handled as the ratio of img[0].size to dpm->dots.
				if ((img[0].size.y == 200) ||
					(img[0].size.y == 240)) {

					vga_mode->flags |= VGA_CMF_LINEDOUBLE;
					dpm->rclk.div = 2;
				}
				
				if ((img[0].size.x == 640) || 
					(img[0].size.x == 720)) {

					vga_mode->flags |= VGA_CMF_DOUBLEWORD;
					dpm->rclk.mul = 2;
				}
			}
		
			break;

		default:
			KRN_DEBUG(1, "Common attributes %.8x not supported", 
				img[0].cam);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		maxsize.x = img[0].size.x;
		maxsize.y = img[0].size.y;
		
		shift = 0;
		switch (bpd) {

		case  1:	shift++;	/* fall through	*/
		case  2:	shift++;	/* fall through */
		case  4:	shift++;	/* fall through	*/
		case  8:	shift++;	/* fall through	*/
			pgm = (pgm << shift) - 1;
			break;			

		default:
			KRN_DEBUG(0, "%i bpd not supported", bpd);
			return -KGI_ERRNO(CHIPSET, FAILED);
		}
		
		KRN_DEBUG(2, "bpd = %i, shift = %i", bpd, shift);
		
		lclk = (cmd == KGI_TC_PROPOSE)
			? dpm->dclk : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		
		KRN_DEBUG(2, "lclk = %i", lclk);
	}

	if ((dpm->flags & KGI_DPF_TP_MASK) == KGI_DPF_TP_LRTB_I1) {

		KRN_DEBUG(1, "Interlaced modes not supported.");
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}
	
	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(2, "KGI_TC_PROPOSE:");
		
		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);
		
		dpm->lclk.mul = dpm->lclk.div =
		dpm->rclk.mul = dpm->rclk.div = 1;
		
		if (img[0].fam == KGI_AM_TEXT) {

			KRN_DEBUG(2, "Proposing textmode");
			KRN_ASSERT(text.x && text.y);
			
			if (!(text.x == 0 || text.y == 0)) {

				maxsize.x = vga->chipset.maxdots.x / text.x;
				maxsize.y = vga->chipset.maxdots.y / text.y;

			} else {

				maxsize.x = vga->chipset.maxdots.x;
				maxsize.y = vga->chipset.maxdots.y;
			}
			
			KRN_DEBUG(2, "text grid %ix%i, maxsize %ix%i, "
				"maxdots %ix%i", text.x, text.y,
				maxsize.x, maxsize.y, vga->chipset.maxdots.x,
				vga->chipset.maxdots.y);
		} else {

			while ((img[0].size.x * dpm->rclk.mul) < 640) {

				dpm->rclk.mul++;
			}

			while ((img[0].size.y * dpm->lclk.div) < 400) {

				dpm->rclk.div++;
			}
			
			KRN_DEBUG(2, "rclk.[mul,div] = [%i,%i]", 
				dpm->rclk.mul, dpm->rclk.div);
		}
		
		/*	If virt.x and virt.y are zero, default to (size.x, max)
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			KRN_DEBUG(2, "virt.x and virt.y == 0; "
				"defaulting to (%i, max)", img[0].size.x);
			
			img[0].virt.x = img[0].size.x;
		}
		
		if (0 == img[0].virt.x) {

			KRN_DEBUG(2, "virt.x == 0");
			
			img[0].virt.x = vga->chipset.memory / 
				(bpp * img[0].virt.y);
			
			if (img[0].virt.x > maxsize.x) {

				KRN_DEBUG(2, "virt.x > maxsize.x");
				
				img[0].virt.x = maxsize.x;
			}
		}
		
		if (0 == img[0].virt.y) {

			KRN_DEBUG(2, "virt.y == 0");
			
			img[0].virt.y = vga->chipset.memory /
				(bpp * img[0].virt.x);

			if (img[0].virt.y > maxsize.y) {

				KRN_DEBUG(2, "virt.y (%i) > maxsize.y (%i)",
					img[0].virt.y, maxsize.y);
				
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

		if (img[0].fam == KGI_AM_TEXT) {

			if ((img[0].virt.x * img[0].virt.y * bpp) > 
				8 * vga->textmemory) {

				KRN_DEBUG(1, "No text memory (%i bytes) "
					"for %ix%i", 
					img[0].virt.x * img[0].virt.y * bpp,
					img[0].virt.x, img[0].virt.y);
				return -KGI_ERRNO(CHIPSET, NOMEM);
			}

			if (((text.x != 8) && (text.x != 9)) || 
				(text.y < 1) || (text.y > 32)) {

				KRN_DEBUG(1, "Invalid text font size (%ix%i)",
					text.x, text.y);
				return -KGI_ERRNO(CHIPSET, NOSUP);
			}

			dpm->dots.x = img[0].size.x * text.x;
			dpm->dots.y = img[0].size.y * text.y;

		} else {

			if ((img[0].virt.x * img[0].virt.y * bpp) >
				vga->gfxmemory) {

				KRN_DEBUG(1, "No graphics memory (%i bytes) "
					"for %ix%i",
					img[0].virt.x * img[0].virt.y *bpp,
					img[0].virt.x, img[0].virt.y);
				return -KGI_ERRNO(CHIPSET, NOMEM);
			}
			
			if ((img[0].size.x != 320) && (img[0].size.x != 360) &&
			    (img[0].size.x != 640) && (img[0].size.x != 720)) {

				KRN_DEBUG(1, "Invalid screen width %i",
					img[0].size.x);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
			
			if ((img[0].size.y != 200) && (img[0].size.y != 240) &&
			    (img[0].size.y != 400) && (img[0].size.y != 480)) {

				KRN_DEBUG(1, "Invalid screen height %i",
					img[0].size.y);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
			
			dpm->dots.x = img[0].size.x;
			dpm->dots.y = img[0].size.y;
		}
		
		KRN_DEBUG(2, "Dotport mode with %ix%i dots",
			dpm->dots.x, dpm->dots.y);
		
		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(2, "KGI_TC_LOWER:");
		
/*		dpm->lclk.mul = 1;
**		dpm->lclk.div = 1;
**		dpm->rclk.mul = 1;
*/		dpm->rclk.div = 1;		

		if (dpm->dclk < vga->chipset.dclk.min) {

			KRN_DEBUG(1, "%i Hz DCLK is too low.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk > vga->chipset.dclk.max) 
			? vga->chipset.dclk.max : dpm->dclk;

		KRN_DEBUG(1, "Before adjust_timing(), DCLK = %i Hz", dpm->dclk);

		vga_chipset_adjust_timing(vga_mode, img);
		
		KRN_DEBUG(1, "After adjust_timing(), DCLK = %i Hz", dpm->dclk);

		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(2, "KGI_TC_RAISE:");

/*		dpm->lclk.mul = 1;
**		dpm->lclk.div = 1 + pgm;
**		dpm->rclk.mul = 1;
*/		dpm->rclk.div = 1;

		if (dpm->dclk > vga->chipset.dclk.max) {

			KRN_DEBUG(1, "%i Hz DCLK is too high.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk < vga->chipset.dclk.min) 
			? vga->chipset.dclk.min : dpm->dclk;
		
		KRN_DEBUG(1, "Before adjust_timing(), DCLK = %i Hz", dpm->dclk);
		
		vga_chipset_adjust_timing(vga_mode, img);
		
		KRN_DEBUG(1, "After adjust_timing(), DCLK = %i Hz", dpm->dclk);
		
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(2, "KGI_TC_CHECK:");

		if (KGI_AM_TEXT == dpm->dam) {

			KRN_DEBUG(2, "Checking a textmode...");
			
			if (kgim_strcmp(dpm->bpda, vga_chipset_448)) {

				KRN_DEBUG(1, "dot port attributes %.8x "
					"not supported", dpm->dam);
				return -KGI_ERRNO(CHIPSET, INVAL);
			}
			
			if (((8 != text.x) && (9 != text.x)) || 
				((text.y < 1) || (32 < text.y))) {

				KRN_DEBUG(1, "font size %ix%i not supported",
					text.x, text.y);
				return -KGI_ERRNO(CHIPSET, INVAL);
			}

			if ((img[0].virt.x * img[0].virt.y * bpp) < 
				vga->textmemory) {

				KRN_DEBUG(1, "not enough text memory for "
					"%ix%i virt.", img[0].virt.x,
					img[0].virt.y);
				return -KGI_ERRNO(CHIPSET, INVAL);
			}

		} else {

			KRN_DEBUG(2, "Checking a graphics mode...");
			
/*			if (kgim_strcmp(dpm->bpda, vga_chipset_i8)) {
**
**				KRN_DEBUG(1, "dot port attributes %.8x "
**					"not supported", dpm->dam);
**				return -KGI_ERRNO(CHIPSET, INVAL);
**			}
*/			
			/* FIXME: Check something here */
		}
			
		if ((dpm->dclk > vga->chipset.dclk.max) || 
			(dpm->dclk < vga->chipset.dclk.min)) {

			KRN_DEBUG(1, "DCLK %i Hz is out of limits.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}


#		define TM(X) 						\
			((vga->chipset.maxdots.x < crt_mode->x.X) ||	\
			 (vga->chipset.maxdots.y < crt_mode->y.X))

			if (TM(width) || TM(blankstart) || TM(syncstart) ||
				TM(syncend) || TM(blankend) || TM(total)) {
	
				KRN_DEBUG(1, "timing check failed");
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}

#		undef TM

		vga_chipset_adjust_timing(vga_mode, img);
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/*	Now everthing is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	KRN_DEBUG(2, "Setting up for mode [%.8x@%ix%i] %ix%i (%ix%i)", 
		  dpm->dam, dpm->dots.x, dpm->dots.y, img[0].size.x, 
		  img[0].size.y, img[0].virt.x, img[0].virt.y);

	mul = crt_mode->in.rclk.mul;

	/*
	**	Set default values
	*/
	
	/*	Text16 framebuffer region
	*/
	vga_mode->text16fb.meta       	= vga;
	vga_mode->text16fb.meta_io    	= vga_io;
	vga_mode->text16fb.type		= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->text16fb.prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->text16fb.name		= "text16 framebuffer";
	vga_mode->text16fb.access	= 8 + 16 + 32 + 64;
	vga_mode->text16fb.align	= 0;
	vga_mode->text16fb.size		= 64 KB; /* FIXME */
	vga_mode->text16fb.win.size	= 32 KB;
	vga_mode->text16fb.win.bus	= vga_io->text16fb.base_bus;
	vga_mode->text16fb.win.phys	= vga_io->text16fb.base_phys;
	vga_mode->text16fb.win.virt	= vga_io->text16fb.base_virt;
	vga_mode->text16fb.SetOffset	= vga_text16_fb_set_offset; /* FIXME */
	
	/*	Graphics framebuffer region
	*/
	vga_mode->pixelfb.meta		= vga;
	vga_mode->pixelfb.meta_io	= vga_io;
	vga_mode->pixelfb.type		= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->pixelfb.prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->pixelfb.name		= "pixel framebuffer";
	vga_mode->pixelfb.access	= 8 + 16 + 32 + 64;
	vga_mode->pixelfb.align		= 0;
	vga_mode->pixelfb.size		= 128 KB;
	vga_mode->pixelfb.win.size	= 128 KB;
	vga_mode->pixelfb.win.bus	= vga_io->pixelfb.base_bus;
	vga_mode->pixelfb.win.phys	= vga_io->pixelfb.base_phys;
	vga_mode->pixelfb.win.virt	= vga_io->pixelfb.base_virt;
	vga_mode->pixelfb.SetOffset	= vga_text16_fb_set_offset;
	
	if (img[0].fam == KGI_AM_TEXT) {

		KRN_DEBUG(2, "Checking TEXT16 mode");
		
		vga_mode->kgim.Command  = vga_text16_command;

/*		vga_mode->MISC		= 0x23;
**		vga_mode->MISC = VGA_MISC_COLOR_IO | VGA_MISC_ENB_RAM | 
**			VGA_MISC_PAGE_SEL; 
*/
		/*	FIXME: Set the default values procedurally
		*/
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
		vga_mode->text16fb_size	= img[0].virt.x * img[0].virt.y;

		/*	Setup bits per pixel and logical screen width
		*/
		foo = img[0].virt.x / 2;
	
		vga_mode->CRT[0x13] |= foo & VGA_CR13_LOGICALWIDTH_MASK;
		vga_mode->CRT[0x14] |= VGA_CR14_UNDERLINE_MASK;
		vga_mode->CRT[0x17] = VGA_CR17_CGA_ADDR_WRAP | 
			VGA_CR17_ENABLE_SYNC | VGA_CR17_CGA_BANKING | 
			VGA_CR17_HGC_BANKING;

		vga_mode->SEQ[0x02] = VGA_SR02_PLANE0 | VGA_SR02_PLANE1;

		vga_mode->GRC[0x05] |= VGA_GR5_ODD_EVEN;
		vga_mode->GRC[0x06] |= VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE;

		switch ((DCLKdiv = img[0].out->dots.x / img[0].size.x)) {

		case 8:
			KRN_DEBUG(2, "DCLKdiv = %i", DCLKdiv);
			vga_mode->SEQ[0x01] |= VGA_SR01_8DOT_CHCLK;
			break;

		case 9:
			KRN_DEBUG(2, "DCLKdiv = %i", DCLKdiv);
			break;

		default:
			KRN_INTERNAL_ERROR;
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		vga_mode->CRT[0x09] |= img[0].out->dots.y/img[0].size.y - 1;
		vga_mode->ATC[0x13] = (DCLKdiv == 9) ? 0x08 : 0x00;

/*		if (mode->magnify.y - 1) {
**
**			vga_mode->CRT[VGA_CRT_CHARHEIGHT] |=
**				VGA_CR09_CHARHEIGHT_MASK;
**		}
*/
	} else {

		/*	Not a textmode
		*/
		KRN_DEBUG(2, "Checking non-TEXT16 mode");
		
		vga_mode->kgim.Command  = vga_text16_command;
		
		vga_mode->orig_dot_x	= 0;
		vga_mode->orig_dot_y	= 0;
		vga_mode->orig_offs	= 0;
		vga_mode->pixelfb_size	= img[0].virt.x * img[0].virt.y;

		/* Setup bits per pixel and logical screen width */
		foo = img[0].virt.x;
		
		DCLKdiv = 8;

		vga_mode->CRT[VGA_CRT_MODE] = 
			VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING | 
			VGA_CR17_ENABLE_SYNC | VGA_CR17_CGA_ADDR_WRAP;

		kgim_memcpy(vga_mode->ATC, vga_default_ATC, sizeof(vga_mode->ATC));
		
		vga_mode->SEQ[VGA_SEQ_WPLANE] |= VGA_SR02_PLANE_MASK;
		vga_mode->SEQ[VGA_SEQ_FONT] = 0x00;

		vga_mode->SEQ[VGA_SEQ_MEMMODE] |= VGA_SR04_256K_ACCESS;

		vga_mode->GRC[VGA_GRC_MMAP_MODE] |= 
		  VGA_GR6_GRAPHMODE | VGA_GR6_MAP_A0_64 | VGA_GR6_MAP_MASK;

		vga_mode->CRT[VGA_CRT_HBLANKEND] |= VGA_CR03_IS_VGA;
		
		/*	FIXME: Handle 4bpp and 1bpp modes
		*/

/*		switch (mode->request.graphtype) 
*/		{
#if 0
		case KGIGT_1BIT:
		case KGIGT_4BIT:
			if (mode->request.graphtype == KGIGT_1BIT) {

				vga_mode->SEQ[VGA_SEQ_WPLANE] |=
					SR02_PLANE_MASK;
				/*	Enable plane 0 for CPU
				*/;
				GRCBITS(0x0e, GR01_DATA_MASK, GRC_ENABLE);
				GRCBITS(0x00, GR04_PLANEMASK, GRC_READPLANE);
				GRCBITS(GR05_WRITEMODE0, 0x03, GRC_MODECONTROL); 
			} else {

				/*	Enable all planes for set/reset 
				**	(color banking)
				*/
				GRCBITS(0x0f, GR01_DATA_MASK, GRC_ENABLE);
				GRCBITS(GR05_WRITEMODE3, 0x03, GRC_MODECONTROL); 
			}
			
			vga_mode->SEQ[VGA_SEQ_MEMMODE] |= 
				SR04_NO_ODDEVEN | SR04_256K_ACCESS;
			vga_mode->SEQ[VGA_SEQ_CLOCK] |= SR01_8DOT_CHCLK;
			vga_mode->CRT[CRT_MODE] |= VGA_CR17_BYTE_MODE;
			vga_mode->ATC[ATC_MODECONTROL] |= AR10_GRAPHICS;

			break;
#endif
/*		case KGIGT_8BIT:
*/
			/*	Note that this implies multiply.x == 2
			*/
			vga_mode->SEQ[VGA_SEQ_CLOCK] |= VGA_SR01_8DOT_CHCLK;		
			vga_mode->SEQ[VGA_SEQ_WPLANE] |= VGA_SR02_PLANE_MASK;
		
			if (img[0].virt.x == 320 && img[0].virt.y == 200) {

				/*	Planar
				*/
				vga_mode->SEQ[VGA_SEQ_MEMMODE] |=
					VGA_SR04_NO_ODDEVEN | VGA_SR04_CHAINED;
				vga_mode->CRT[VGA_CRT_UNDERLINE] |=
					VGA_CR14_DOUBLEWORDMODE;
			} else { 

				/*	Mode X
				*/
/*				SEQBITS(SR04_NO_ODDEVEN, 0x0f, VGA_SEQ_MEMMODE);
**				vga_mode->CRT[CRT_MODE] |= VGA_CR17_BYTE_MODE;
*/			}

#if 0
			vga_mode->ATC[VGA_ATC_MODECONTROL] &= ~ 
				(VGA_AR10_GRAPHICS | VGA_AR10_MONOMODE | 
				VGA_AR10_ENB_BLINK | 0x10 /* reserved (0) */ |
				VGA_AR10_SELECT_B54);
#endif
			vga_mode->ATC[VGA_ATC_MODECONTROL] = 0;
			
			vga_mode->ATC[VGA_ATC_MODECONTROL] =
				VGA_AR10_GRAPHICS | VGA_AR10_8BIT_MODE |
				VGA_AR10_TOP_PAN;

			vga_mode->ATC[VGA_ATC_BORDER] = 0;
			
			vga_mode->ATC[VGA_ATC_HPAN] = 0;
			vga_mode->ATC[VGA_ATC_PIXELPAD] &= ~ VGA_AR14_DATA_MASK;

			vga_mode->GRC[VGA_GRC_MODECONTROL] |=
				VGA_GR5_8BIT_COLOR;
			vga_mode->GRC[VGA_GRC_BITMASK] = 0xff; /* Enable all bits */
			
/*			break;
**			
**		default:
**			KRN_DEBUG(0, "Unsupported display type!");
**			return -KGI_ERRNO(CHIPSET, UNKNOWN);
*/		}

	}

	/*	Setup horizontal timing
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

	/*	Setup vertical timing
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
	
	if (vga_mode->flags & VGA_CMF_LINEDOUBLE) {

		vga_mode->CRT[0x09] |= VGA_CR09_DOUBLESCAN;
	}
	
	if (vga_mode->flags & VGA_CMF_8BPPMODE) {

		vga_mode->ATC[0x10] |= VGA_AR10_8BIT_MODE;
	}
	
	if (vga_mode->flags & VGA_CMF_DOUBLEWORD) {

		vga_mode->CRT[0x17] = VGA_CR17_WORDMODE;
	}
	
	return KGI_EOK;
}

kgi_resource_t *vga_chipset_mode_resource(vga_chipset_t *vga, 
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index)
{
	KRN_DEBUG(2, "vga_chipset_mode_resource(index = %d)", index);
	
	switch (index) {

	case 0:	return (kgi_resource_t *) &vga_mode->text16fb;
	case 1:	return (kgi_resource_t *) &vga_mode->pixelfb;
		
	default:
		KRN_DEBUG(2, "Unknown resource requested");
		return NULL;
	}
}

void vga_chipset_mode_prepare(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "vga_chipset_mode_prepare()");

#if 0
	VGA_SEQ_OUT8(vga_io, VGA_SEQ_IN8(vga_io, 0x01) | VGA_SR01_DISPLAY_OFF, 0x01);
#endif
}

void vga_chipset_mode_enter(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_chipset_mode_enter()");

	VGA_MISC_OUT8(vga_io,
		(VGA_MISC_IN8(vga_io) & VGA_MISC_CLOCK_MASK) | vga_mode->MISC);
	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);

#define	VGA_SET(subsys)							\
	for (i = 0; i < VGA_##subsys##_REGS; i++) {			\
									\
		VGA_##subsys##_OUT8(vga_io, vga_mode->subsys[i], i);	\
	}

/*     	VGA_SET(SEQ);
*/
#if 1
	for (i = 0; i < VGA_SEQ_REGS; i++) {

		switch (i) {

		case 0x00:
		case 0x01:
		case 0x02:
/*		case 0x03:
*/		case 0x04:
		case 0x05:
		case 0xff:
			VGA_SEQ_OUT8(vga_io, vga_mode->SEQ[i], i);
		default:
			break;
		}
	}
#endif
	
	VGA_CRT_OUT8(vga_io, VGA_CRT_IN8(vga_io, 0x11) & ~VGA_CR11_LOCKTIMING,
		0x11);
	VGA_SET(CRT);
	VGA_SET(GRC);
/*     	VGA_SET(ATC);
*/
#undef	VGA_SET
	
/*	*((kgi_u32_t *) vga_mode->fb.win.virt) = 0x12345678;
*/
}
