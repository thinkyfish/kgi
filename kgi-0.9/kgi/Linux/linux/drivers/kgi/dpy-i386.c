/* -----------------------------------------------------------------------------
**	i386 boot display code
** -----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** -----------------------------------------------------------------------------
**
**	$Log: dpy-i386.c,v $
**	Revision 1.2  2000/06/02 09:17:17  seeger_s
**	- fixed broken compile with SPLASH screen enabled
**	
**	Revision 1.1.1.1  2000/04/18 08:50:47  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#define	DEBUG_LEVEL	2

#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#include <linux/config.h>
#include <linux/version.h>
#include <asm/io.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/string.h>
#include <linux/ascii.h>
#include <linux/console.h>
#include <linux/kdev_t.h>

#define	KGI_SYS_NEED_IO
#include <kgi/kgi.h>
#include <linux/kgii.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
extern struct wait_queue *keypress_wait;
#else
extern wait_queue_head_t keypress_wait;
#endif

#define	CONFIG_VT_CONSOLE

#ifdef CONFIG_KGI_SPLASH

/*	This a Quick&Dirty(TM) Hack to be able to slowdown the printk 
**	to follow the early kernel boot messages in case there are 
**	kernel-Ooops's before we have scrollback.
*/
static kgi_u32_t msg[] = {
	0x44333212, 0x12111111, 0x12121112, 0x11121112, 0x12111111,
	0x12122212, 0x12113444, 0x45454545, 0x55555555, 0x55555555,
	0x43433323, 0x33222111, 0x11111111, 0x11111111, 0x12211111,
	0x11112111, 0x11112344, 0x45555555, 0x45545454, 0x55555555,
	0x46643636, 0x66366611, 0x61121662, 0x16161212, 0x66622266,
	0x32623262, 0x66626664, 0x46664666, 0x54455666, 0x45656554,
	0x46464626, 0x22262265, 0x61226236, 0x36262223, 0x62263644,
	0x64643464, 0x64446446, 0x46445644, 0x64454644, 0x65636354,
	0x46462646, 0x66366632, 0x62326666, 0x36363412, 0x66643655,
	0x63636465, 0x66456664, 0x46654645, 0x65354666, 0x54646565,
	0x56564633, 0x36464444, 0x63436226, 0x22622222, 0x61233643,
	0x64646364, 0x63456556, 0x46534654, 0x64543644, 0x65464544,
	0x46653626, 0x66463434, 0x66646336, 0x22621212, 0x61122266,
	0x34365632, 0x66646436, 0x36663666, 0x33454666, 0x54463464,
	0x53334242, 0x23444333, 0x33334333, 0x22211111, 0x11111123,
	0x33332122, 0x21223333, 0x34335343, 0x43454545, 0x44434333,
	0x45353544, 0x33344444, 0x34545432, 0x32221211, 0x11111123,
	0x34443302, 0x32323234, 0x45353535, 0x34545454, 0x54444432,
	0x54534454, 0x54233410, 0x10320202, 0x32444321, 0x11111122,
	0x31211202, 0x00102223, 0x33344444, 0x44444444, 0x44445443,
	0x45454545, 0x31331066, 0x66666666, 0x60023032, 0x11122006,
	0x66666666, 0x66112033, 0x22366664, 0x34343444, 0x44444432,
	0x55445533, 0x21066666, 0x11101116, 0x66661101, 0x12036666,
	0x61123033, 0x66666101, 0x32666661, 0x01011212, 0x12121212,
	0x45454442, 0x26666010, 0x10110112, 0x12120012, 0x00666601,
	0x00120303, 0x11121101, 0x32666661, 0x01011212, 0x12121212,
	0x44444432, 0x26666111, 0x01101666, 0x66661102, 0x30666613,
	0x00330166, 0x66666310, 0x31666661, 0x01111122, 0x21222122,
	0x44343432, 0x21166666, 0x11111012, 0x66660102, 0x12206666,
	0x61111101, 0x16666103, 0x21666661, 0x11121212, 0x12323212,
	0x33222322, 0x20011166, 0x66666666, 0x66632344, 0x31110206,
	0x66666666, 0x66662131, 0x31666660, 0x01101111, 0x11233322,
	0x12221212, 0x00101210, 0x12001222, 0x33334444, 0x53311312,
	0x22311332, 0x12121202, 0x02020201, 0x11111111, 0x11111211,
	0x11222121, 0x11211111, 0x11111233, 0x33455555, 0x45433344,
	0x33333321, 0x11111121, 0x11011111, 0x11111011, 0x11111111,
	0x16661616, 0x16661116, 0x66126336, 0x65564646, 0x66445666,
	0x36363636, 0x11621662, 0x12166611, 0x66321166, 0x61116612,
	0x11612626, 0x26111116, 0x11636365, 0x45565655, 0x65554563,
	0x26262626, 0x61616111, 0x11236226, 0x11611161, 0x16161161,
	0x34643666, 0x36623236, 0x66446465, 0x66466655, 0x64443363,
	0x16661616, 0x16616166, 0x12116216, 0x11611161, 0x16161161,
	0x45645656, 0x56444446, 0x44656564, 0x46565644, 0x64222262,
	0x16261616, 0x21616116, 0x11116116, 0x11612161, 0x26161161,
	0x55655646, 0x56663456, 0x54656456, 0x63565632, 0x62121262,
	0x16161616, 0x12611660, 0x11116111, 0x66121166, 0x62126611,
	0x55555555, 0x55554554, 0x55555555, 0x55555443, 0x23242124,
	0x21414333, 0x43333333, 0x33333332, 0x23322323, 0x22312111,
	0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555,
	0x55555545, 0x44544544, 0x55444454, 0x44444454, 0x44422424
};

static kgi_rgb_color_t msg_col[8] = 
{
	{ 0, 0, 0}, { 0, 0,13}, { 0, 0,26}, { 0, 0,38},
	{ 0, 0,51}, { 0, 0,63}, {24, 0,24}, {63,63, 0}
};

static kgi_rgb_color_t def_col[8] =
{
	{ 0, 0, 0}, { 0, 0,42}, { 0,42, 0}, { 0,42,42},
	{42, 0, 0}, {42, 0,42}, {42,42, 0}, {42,42,42}
};

static kgi_rgb_color_t curr_col[8] = { };

static void set_col(void)
{
	int i;

	for (i = 0; i < 8; i++) {

		outb_p((i == 6) ? 0x14 : i, 0x3C8);
		outb_p(curr_col[i].r, 0x3C9);
		outb_p(curr_col[i].g, 0x3C9);
		while (inb_p(0x3DA) & 1);
		while (!(inb_p(0x3DA) & 1));
		outb_p(curr_col[i].b, 0x3C9);
	}

	if (inb_p(0x3DA) & 8) {
		while (inb_p(0x3DA) & 8);
	}

	while (!(inb_p(0x3DA) & 8));
}

static void show_msg(ushort *fb, kgi_u_t sizex, kgi_u_t sizey)
{
	int line = 0, firstrow = (sizex - 80)/2, i;

	fb += ((sizey - 25)/2)*sizex;

	curr_col[7] = def_col[7];
	set_col();

	while (line < 25) {

		int row;
		ushort *p = fb + firstrow;

		for (row = 0; row < 10; row++) {
			int attr = msg[line*10+row];

			#define SETBG(x) *p = (*p & 0x00FF) | 0x0700\
						| ((x) & 0xF000); p++

			SETBG(attr>>16);	SETBG(attr>>12);
			SETBG(attr>>8);		SETBG(attr>>4);
			SETBG(attr);		SETBG(attr<<4);
			SETBG(attr<<8);		SETBG(attr<<12);
		}

		fb += sizex;
		line++;
	}

	for (i = 0; i < 100; i++) {

		int reg;

		for (reg = 0; reg < 7; reg++) {

			curr_col[reg].r = msg_col[reg].r*i/100;
			curr_col[reg].g = msg_col[reg].g*i/100;
			curr_col[reg].b = msg_col[reg].b*i/100;
		}

		curr_col[7].r = (msg_col[7].r-def_col[7].r)*i/100+def_col[7].r;
		curr_col[7].g = (msg_col[7].g-def_col[7].g)*i/100+def_col[7].g;
		curr_col[7].b = (msg_col[7].b-def_col[7].b)*i/100+def_col[7].b;
		set_col();
	}

	for (i = 0; i < 100; i++) {
		curr_col[6].r = (63 - msg_col[6].r)*i/100 + msg_col[6].r;
		curr_col[6].g = (63 - msg_col[6].g)*i/100 + msg_col[6].g;
		curr_col[6].b = ( 0 - msg_col[6].b)*i/100 + msg_col[6].b;
		set_col();
	}

	for (i = 0; i < 100; i++) {
		curr_col[6].r = (msg_col[6].r - 63)*i/100 + 63;
		curr_col[6].g = (msg_col[6].g - 63)*i/100 + 63;
		curr_col[6].b = (msg_col[6].b -  0)*i/100 +  0;
		set_col();
	}
}

static void hide_msg(ushort *fb, kgi_u_t sizex, kgi_u_t sizey)
{
	int i;

	for (i = 100; i >= 0; i--) {

		int reg;

		for (reg = 0; reg < 7; reg++) {
			curr_col[reg].r = msg_col[reg].r*i/100;
			curr_col[reg].g = msg_col[reg].g*i/100;
			curr_col[reg].b = msg_col[reg].b*i/100;
		}

		curr_col[7].r = (msg_col[7].r-def_col[7].r)*i/100+def_col[7].r;
		curr_col[7].g = (msg_col[7].g-def_col[7].g)*i/100+def_col[7].g;
		curr_col[7].b = (msg_col[7].b-def_col[7].b)*i/100+def_col[7].b;

		set_col();
	}

	i = sizex*sizey;

	while (i--) {
		*fb = (*fb & 0x00FF) | 0x0700;
		fb++;
	}

	for (i = 0; i < 8; i++) {
		curr_col[i] = def_col[i];
	}

	set_col();
}
#endif	/* #ifdef CONFIG_KGI_SPLASH	*/

/* -----------------------------------------------------------------------------
**	display methods
** -----------------------------------------------------------------------------
*/
#if 0
/*	reset() simply clears the framebuffer.
*/
static void text16_reset(kgi_display_t *dpy)
{
/*	memset_io(dpy->fb.pbuf, 0, dpy->mode.fb_size);  */
}
#endif

/* ----------------------------------------------------------------------------
**	kernel interface
** ----------------------------------------------------------------------------
**/

typedef struct
{
	kgi_dot_port_mode_t	dpm;

	kgi_marker_t		pointer_ctrl;
	kgi_u_t			pointer_mask, pointer_old;

	kgi_marker_t		cursor_ctrl;
	kgi_u_t			cursor_mask, cursor_old;

	kgi_text16_t		text16;

} text16_mode_t;

typedef struct
{
	kgi_display_t		dpy;
	kgi_mode_t		mode;
	kgi_ucoord_t		screen_size;
	kgi_mmio_region_t	fb;
	kgi_accel_t		accel;

} text16_display_t;

/*	text16 operations
*/
static void text16_pointer_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	text16_mode_t  *text16 = ptr->meta;
	register kgi_u16_t *fb = ptr->meta_io;
	register kgi_u_t new;

	x /= text16->text16.cell.x;
	y /= text16->text16.cell.y;
	new = x  +  y * text16->text16.virt.x;

	KRN_ASSERT((x < text16->text16.size.x) && (y < text16->text16.size.y));

	if (text16->pointer_old & 0x8000) {

		mem_out16(text16->pointer_old >> 16, 
			(mem_vaddr_t) (fb + (text16->pointer_old & 0x7FFF)));
	}

	fb += new;
	if ((text16->cursor_old & 0x8000) &&
		((text16->cursor_old & 0x7FFF) == (new & 0x7FFF))) {

		new |= text16->cursor_old & 0xFFFF0000;

	} else {

		new |= mem_in16((mem_vaddr_t) fb) << 16;
	}

	text16->pointer_old = new | 0x8000;
	mem_out16(mem_in16((mem_vaddr_t) fb) ^ text16->pointer_mask,
		(mem_vaddr_t) fb);
}

static void text16_pointer_hide(kgi_marker_t *ptr)
{
	text16_mode_t *text16 = ptr->meta;

	if (text16->pointer_old & 0x8000) {

		kgi_u16_t *fb = ptr->meta_io;
		mem_out16(text16->pointer_old >> 16,
			(mem_vaddr_t) (fb + (text16->pointer_old & 0x7FFF)));
	}
	text16->pointer_old = 0;
}

#define	text16_pointer_undo	text16_pointer_hide


static void text16_cursor_show(kgi_marker_t *cur, kgi_u_t x, kgi_u_t y)
{
	text16_mode_t *text16 = cur->meta;
	register kgi_u16_t *fb = cur->meta_io;

	register kgi_u_t new = x  +  y * text16->text16.virt.x;

	KRN_ASSERT((x < text16->text16.size.x) && (y < text16->text16.size.y));

	if (text16->cursor_old & 0x8000) {

		mem_out16(text16->cursor_old >> 16,
			(mem_vaddr_t) (fb + (text16->cursor_old & 0x7FFF)));
	}

	fb += new;
	if ((text16->pointer_old & 0x8000) &&
		((text16->pointer_old & 0x7FFF) == (new & 0x7FFF))) {

		new |= text16->pointer_old & 0xFFFF0000;

	} else {

		new |= mem_in16((mem_vaddr_t) fb) << 16;
	}

	text16->cursor_old = new | 0x8000;
	mem_out16(mem_in16((mem_vaddr_t) fb) ^ text16->cursor_mask,
		(mem_vaddr_t) fb);
}

static void text16_cursor_hide(kgi_marker_t *cur)
{
	text16_mode_t *text16 = cur->meta;

	if (text16->cursor_old & 0x8000) {

		ushort *fb = cur->meta_io;
		mem_out16(text16->cursor_old >> 16,
			(mem_vaddr_t) (fb + (text16->cursor_old & 0x7FFF)));
	}
	text16->cursor_old = 0;
}

#define	text16_cursor_undo	text16_cursor_hide


static void text16_put_text16(kgi_text16_t *text16,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
	text16_mode_t *text16_mode = text16->meta;
	register kgi_u16_t *fb = text16->meta_io;
	KRN_ASSERT(offset < 4096);
	KRN_ASSERT(offset + count < 4096);
	KRN_ASSERT(! (text16_mode->cursor_old & 0x00008000));
	KRN_ASSERT(! (text16_mode->pointer_old & 0x00008000));
	mem_put16((mem_vaddr_t) (fb + offset), text, count);
}


static kgi_error_t text16_display_command(kgi_display_t *dpy,
	kgi_u_t cmd, void *in, void **out, kgi_size_t *out_size)
{
	*out = NULL;
	*out_size = 0;
	return -EINVAL;
}

static void text16_inc_refcount(kgi_display_t *dpy)
{
	KRN_ASSERT(dpy);
	KRN_DEBUG(2, "text16 display refcount increment");
}

static void text16_dec_refcount(kgi_display_t *dpy)
{
	KRN_ASSERT(dpy);
	KRN_DEBUG(2, "text16 display refcount decrement");
}

#define	print_img_mode(lvl,img)	KRN_DEBUG(lvl,				\
	"%ix%i (%ix%i), %i frames, fam=%.8x, bpam (IFTB) = %i%i%i%i",	\
	img[0].size.x, img[0].size.y, img[0].virt.x, img[0].virt.y,	\
	img[0].frames, img[0].fam, img[0].bpfa[0], img[0].bpfa[1],	\
	img[0].bpfa[2], img[0].bpfa[3])

static int text16_check_mode(kgi_display_t *dpy, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images, void *dev_mode, 
	const kgi_resource_t **r, kgi_u_t rsize)
{
#define	DEFAULT(x)	if (! (x)) { x = text16->mode.x; }
#define	MATCH(x)	((x) == text16->mode.x)
	text16_display_t *text16 = (text16_display_t *) dpy;
	text16_mode_t *devmode = (text16_mode_t *) dev_mode;

	if (images != 1) {

		KRN_DEBUG(2, "%i image layers are not supported.", images);
		return -EINVAL;
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "proposing boot mode");
		print_img_mode(3, img);
		DEFAULT(img[0].virt.x);
		DEFAULT(img[0].virt.y);
		DEFAULT(img[0].size.x);
		DEFAULT(img[0].size.y);
		DEFAULT(img[0].frames);
		DEFAULT(img[0].tluts);
		DEFAULT(img[0].iluts);
		DEFAULT(img[0].aluts);
		if (! img[0].fam) {

			img[0].fam  = text16->mode.img[0].fam;
			img[0].cam  = text16->mode.img[0].cam;
			memmove(img[0].bpfa, text16->mode.img[0].bpfa,
				sizeof(img[0].bpfa));
			memmove(img[0].bpca, text16->mode.img[0].bpca,
				sizeof(img[0].bpca));
		}
		/*	fall through
		*/
	case KGI_TC_CHECK:
		print_img_mode(3, img);
		print_img_mode(3, text16->mode.img);
		if (! (MATCH(img[0].virt.x) && MATCH(img[0].virt.y) &&
			MATCH(img[0].size.x) && MATCH(img[0].size.y) &&
			MATCH(img[0].frames) && MATCH(img[0].tluts) &&
			MATCH(img[0].iluts) && MATCH(img[0].aluts))) {

			KRN_DEBUG(2, "image mode does not match boot mode");
			return -EINVAL;
		}
		if (!MATCH(img[0].fam) || !MATCH(img[0].cam) ||
			strncmp(img[0].bpfa, text16->mode.img[0].bpfa,
				__KGI_MAX_NR_ATTRIBUTES) ||
			strncmp(img[0].bpca, text16->mode.img[0].bpca,
				__KGI_MAX_NR_ATTRIBUTES)) {

			KRN_DEBUG(2, "attributes do not match boot mode");
			return -EINVAL;
		}

		devmode->dpm.dots.x = text16->screen_size.x;
		devmode->dpm.dots.y = text16->screen_size.y;
		devmode->dpm.dam    = img[0].fam;
		devmode->dpm.bpda   = img[0].bpfa;
		img[0].out = &devmode->dpm;
		img[0].flags = KGI_IF_TEXT16;

		if ((1 < rsize) && r) {

			r[0] = (kgi_resource_t *) &text16->fb;
			r[1] = (kgi_resource_t *) &text16->accel;
		}

		/*	text16 control
		*/
		devmode->text16.meta		= devmode;
		devmode->text16.meta_io		= text16->fb.win.virt;
		devmode->text16.type		= KGI_RT_TEXT16_CONTROL;
		devmode->text16.prot		= KGI_PF_DRV_RWS;
		devmode->text16.name		= "text16 control";
		devmode->text16.size.x		= img[0].size.x;
		devmode->text16.size.y		= img[0].size.y;
		devmode->text16.virt.x		= img[0].size.x;
		devmode->text16.virt.y		= img[0].size.y;
		devmode->text16.cell.x		= 8;
		devmode->text16.cell.y		= 14;
		devmode->text16.font.x		= 8;
		devmode->text16.font.y		= 14;
		devmode->text16.PutText16	= text16_put_text16;

		KRN_NOTICE("text16 set up: size %ix%i, virt %ix%i",
			devmode->text16.size.x, devmode->text16.size.y,
			devmode->text16.virt.x, devmode->text16.virt.y);

		/*	cursor control
		*/
		devmode->cursor_mask = (img[0].fam & KGI_AM_BLINK)
			? 0x7700 : 0xFF00;
		devmode->cursor_old = 0;

		devmode->cursor_ctrl.meta	= devmode;
		devmode->cursor_ctrl.meta_io	= text16->fb.win.virt;
		devmode->cursor_ctrl.type	= KGI_RT_CURSOR_CONTROL;
		devmode->cursor_ctrl.prot	= KGI_PF_DRV_RWS;
		devmode->cursor_ctrl.name	= "cursor control";
		devmode->cursor_ctrl.size.x	= 1;
		devmode->cursor_ctrl.size.y	= 1;
		devmode->cursor_ctrl.Show	= text16_cursor_show;
		devmode->cursor_ctrl.Hide	= text16_cursor_hide;
		devmode->cursor_ctrl.Undo	= text16_cursor_undo;


		/*	pointer control
		*/
		devmode->pointer_mask = (img[0].fam & KGI_AM_BLINK)
			? 0x7700 : 0x7F00;
		devmode->pointer_old = 0;

		devmode->pointer_ctrl.meta	= devmode;
		devmode->pointer_ctrl.meta_io	= text16->fb.win.virt;
		devmode->pointer_ctrl.type	= KGI_RT_POINTER_CONTROL;
		devmode->pointer_ctrl.prot	= KGI_PF_DRV_RWS;
		devmode->pointer_ctrl.name	= "pointer control";
		devmode->pointer_ctrl.size.x	= 1;
		devmode->pointer_ctrl.size.y	= 1;
		devmode->pointer_ctrl.Show	= text16_pointer_show;
		devmode->pointer_ctrl.Hide	= text16_pointer_hide;
		devmode->pointer_ctrl.Undo	= text16_pointer_undo;

		img[0].resource[0]= (kgi_resource_t *) &(devmode->text16);
		img[0].resource[1]= (kgi_resource_t *) &(devmode->cursor_ctrl);
		img[0].resource[2]= (kgi_resource_t *) &(devmode->pointer_ctrl);

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -EINVAL;
	}
#undef	MATCH
#undef	DEFAULT
}

void text16_set_mode(kgi_display_t *dpy, kgi_image_mode_t *img, kgi_u_t images,
	void *dev_mode)
{
	KRN_ASSERT(dpy);
	KRN_ASSERT(img);
	KRN_ASSERT(images == 1);
	KRN_ASSERT(dev_mode);
}


typedef struct 
{
	kgi_accel_context_t	kgi;

	kgi_ascii_t color[64];

} blubber_context_t;

static void blubber_accel_init(kgi_accel_t *accel, void *ctx)
{
	kgi_u8_t *blubber = accel->meta;
	blubber_context_t *blubber_ctx = ctx;

	strncpy(blubber_ctx->color, "initial", sizeof(blubber_ctx->color));
	KRN_DEBUG(0, "%s accel context initialized (%s color)",
		blubber, blubber_ctx->color);
}

static void blubber_accel_done(kgi_accel_t *accel, void *ctx)
{
	kgi_u8_t *blubber = accel->meta;
	blubber_context_t *blubber_ctx = ctx;

	if (accel->ctx == ctx) {

		accel->ctx = NULL;
	}
	KRN_DEBUG(0, "%s accel context destroyed (%s color)",
		blubber, blubber_ctx->color);
}

static void blubber_accel_exec(kgi_accel_t *accel, kgi_accel_buffer_t *buffer)
{
	kgi_ascii_t *action = buffer->aperture.virt;
	blubber_context_t *ctx = accel->ctx;

	if (ctx != buffer->exec_ctx) {

		if (ctx) {

			KRN_DEBUG(0, "%s accel saved %s color", accel->meta,
				ctx ? ctx->color : "no");
		}
		accel->ctx = ctx = buffer->exec_ctx;
		KRN_DEBUG(0, "%s accel loaded %s color", accel->meta,
			ctx ? ctx->color : "no");
	}

	action[(buffer->exec_size > 20) ? buffer->exec_size : 20] = 0;
	KRN_DEBUG(0, "%s accel executed %s with %s color",
		accel->meta, action, ctx->color);
	buffer->exec_state = KGI_AS_IDLE;
}


/* ----------------------------------------------------------------------------
**	text16_malloc_display() allocates and initializes a struct kgi_display
**	that handles the assigned area.
**
**	Note this is a boot display only so some corners have been cut
**	vs. when a KGI driver is present with it's own text mode scroller.
** -----------------------------------------------------------------------------
*/

#define	COLOR	1
#define	MONO	0
kgi_display_t *text16_malloc_display(unsigned long fb, kgi_u_t fb_size,
	kgi_u_t size_x, kgi_u_t size_y, kgi_u_t stride,
	kgi_u_t screen_x, kgi_u_t screen_y, int color)
{
	text16_display_t *text16 = kmalloc(sizeof(*text16), GFP_KERNEL);

	KRN_ASSERT(size_x && size_y && screen_x && screen_y);

	if (text16 == NULL) {

		return NULL;
	}
	memset(text16, 0, sizeof(*text16));

	/*	initialize display struct
	*/
	text16->dpy.revision = KGI_DISPLAY_REVISION;
	sprintf(text16->dpy.vendor, "KGI boot display");
	sprintf(text16->dpy.model,
		"%s text16 @%p", color ? "color" : "monochrome", (void *) fb);
	text16->dpy.flags = 0;
	text16->dpy.mode_size = sizeof(text16_mode_t);
	text16->dpy.Command = text16_display_command;

/*	text16->dpy.priv.priv_u64 = 0; */
	text16->dpy.mode = &text16->mode;
	text16->dpy.id = -1;
	text16->dpy.graphic = 0;
	text16->dpy.IncRefcount = text16_inc_refcount;
	text16->dpy.DecRefcount = text16_dec_refcount;

	text16->dpy.CheckMode = text16_check_mode;
	text16->dpy.SetMode = text16_set_mode;

	text16->dpy.focus = NULL;

	/*	initialize mode struct 
	*/
	text16->mode.revision = KGI_MODE_REVISION;
	text16->mode.dev_mode = NULL;
	text16->mode.resource[0] = (kgi_resource_t *) &text16->fb;
	text16->mode.images = 1;
	text16->mode.img[0].out = NULL;
	text16->mode.img[0].flags = 0;
	text16->mode.img[0].virt.x = stride;
	text16->mode.img[0].virt.y = size_y;
	text16->mode.img[0].size.x = size_x;
	text16->mode.img[0].size.y = size_y;
	text16->mode.img[0].frames = 1;
	text16->mode.img[0].tluts = 0;
	text16->mode.img[0].aluts = 0;
	text16->mode.img[0].ilutm = 0;
	text16->mode.img[0].alutm = 0;
/*	text16->mode.img[0].cam = 0;
**	text16->mode.img[0].bpca = { 0, ... };
*/	if (color) {

		text16->mode.img[0].fam = KGI_AM_COLOR_INDEX |
			KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX;
		text16->mode.img[0].bpfa[0] = 4;	/* bg color */
		text16->mode.img[0].bpfa[1] = 4;	/* fg color */
		text16->mode.img[0].bpfa[2] = 8;	/* texture  */

	} else {

		text16->mode.img[0].fam = KGI_AM_COLOR_INDEX |
			KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX |
			KGI_AM_BLINK;
		text16->mode.img[0].bpfa[0] = 3;	/* index    */
		text16->mode.img[0].bpfa[1] = 4;	/* fg color */
		text16->mode.img[0].bpfa[2] = 8;	/* texture  */
		text16->mode.img[0].bpfa[3] = 1;	/* blink    */
	}

	/*	initialize mmio struct
	*/
	text16->fb.meta         = &text16->dpy;
	text16->fb.type		= KGI_RT_MMIO_FRAME_BUFFER;
	text16->fb.prot		= KGI_PF_APP_RWS | KGI_PF_LIB_RWS | KGI_PF_DRV_RWS;
	text16->fb.name		= "frame buffer";

	text16->fb.access	= 8 + 16 + 32 + 64;
	text16->fb.align	= 8 + 16;
	text16->fb.win.size	= (fb_size + ~PAGE_MASK - 1) & PAGE_MASK;
	text16->fb.win.virt	= phys_to_virt(fb);
	text16->fb.win.bus	= virt_to_bus((void *) fb);
	text16->fb.win.phys	= fb;

	text16->fb.size = text16->fb.win.size;
	text16->fb.offset = 0;
	text16->fb.SetOffset = NULL;

	text16->accel.meta = text16->dpy.model;
	text16->accel.meta_io = NULL;
	text16->accel.type = KGI_RT_ACCELERATOR;
	text16->accel.prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	text16->accel.name = "blubber accel";
	text16->accel.buffers = 2;	
	text16->accel.buffer_size = PAGE_SIZE;
	text16->accel.ctx = NULL;
	text16->accel.ctx_size = sizeof(blubber_context_t);
	text16->accel.exec_queue = NULL;
	text16->accel.Init = blubber_accel_init;
	text16->accel.Done = blubber_accel_done;
	text16->accel.Exec = blubber_accel_exec;

	/*	initialize driver specific stuff
	*/
	text16->screen_size.x = screen_x;
	text16->screen_size.y = screen_y;

	return (kgi_display_t *) text16;
}

void text16_free_display(kgi_display_t *dpy)
{
	kfree(dpy);
}

/* -----------------------------------------------------------------------------
**	boot console for i386
** -----------------------------------------------------------------------------
*/
#ifdef CONFIG_VT_CONSOLE

static int kgi_boot_pos, kgi_boot_need_wrap;
static unsigned short *kgi_boot_fb;

/*	This is a Quick&Dirty (TM) console_print to be able to display printk 
**	messages as early as possible. This is replaced with a console aware
**	implementation as soon as the KGI display manager is fired up and
**	running.
*/
#define	kgi_boot_attr	0x0700

static void kgi_boot_cr(void)
{
	kgi_boot_pos -= ORIG_X;
	kgi_boot_need_wrap = ORIG_X = 0;
}

static void kgi_boot_lf(void)
{
	if (ORIG_Y+1 == ORIG_VIDEO_LINES) {

		int cnt = ORIG_VIDEO_COLS*(ORIG_VIDEO_LINES - 1);

#		ifdef CONFIG_KGI_SPLASH

			unsigned short *d = kgi_boot_fb;
			unsigned short *s = kgi_boot_fb + ORIG_VIDEO_COLS;

			while (cnt--) {

				*d = (*d & 0xFF00) | (*s & 0x00FF);
				d++; s++;
			}

			cnt = ORIG_VIDEO_COLS;
			while (cnt--) {

				*(d++) &= 0xFF00;
			}
#		else
			memcpy(kgi_boot_fb, kgi_boot_fb+ORIG_VIDEO_COLS, cnt*2);
			memset(kgi_boot_fb+cnt, 0, ORIG_VIDEO_COLS*2);
#		endif

	} else {

		ORIG_Y++;
		kgi_boot_pos += ORIG_VIDEO_COLS;
	}
	kgi_boot_need_wrap = 0;
}

static void kgi_boot_console_printk(struct console *cp, const char *s, 
	unsigned count)
{
	static int printing = 0;

	if (printing) {

		return;
	}
	printing = 1;
 
	while (count--) {

		if ((*s == ASCII_LF) || 
		    (*s == ASCII_CR) || kgi_boot_need_wrap) {

			if (*s != ASCII_CR) {

				kgi_boot_lf();
			}

			kgi_boot_cr();

			if ((*s == ASCII_LF) || (*s == ASCII_CR)) {

				s++;
				continue;
			}
		}

#		ifdef CONFIG_KGI_SPLASH

			kgi_boot_fb[kgi_boot_pos] = *s | (kgi_boot_fb[kgi_boot_pos] & 0xFF00);

			/*	slow down to be able to read early boot messages
			**	even when they scroll up.
			*/
			if (ORIG_VIDEO_ISVGA && !(kgi_boot_pos & 0x3)) {

				while (inb(0x3DA) & 8);
				while (!(inb(0x3DA) & 8));
		        }
#		else
			kgi_boot_fb[kgi_boot_pos] = *s | kgi_boot_attr;
#		endif

		s++;
		kgi_boot_pos++;
		ORIG_X++;

		if ((kgi_boot_need_wrap = (ORIG_X == ORIG_VIDEO_COLS))) {

			continue;
		}
	}
	printing = 0;
}

static int kgi_boot_console_wait_key(struct console *cp)
{
	sleep_on(&keypress_wait);
	return 0;
}

static kdev_t kgi_boot_console_device(struct console *cp)
{
	return MKDEV(TTY_MAJOR, 1);
}

static struct console kgi_boot_console =
{
	"tty",				/* name			*/
	kgi_boot_console_printk,	/* write		*/
	NULL,				/* read			*/
	kgi_boot_console_device,	/* device		*/
	kgi_boot_console_wait_key,	/* wait_key		*/
	NULL,				/* unblank		*/
	NULL, 				/* setup		*/
	0,				/* flags		*/
	-1,				/* index		*/
	0,				/* cflag		*/
	NULL				/* next			*/
};

#endif	/* #ifdef CONFIG_VT_CONSOLE */

/* -----------------------------------------------------------------------------
**	kernel interface
** -----------------------------------------------------------------------------
*/

/*	no_scroll() was introduced to disable hardware scrolling for some
**	braille lines. We assume a MDA-like framebuffer anyhow, so this does
**	simply nothing and has become obsolete. I think...
*/	
void no_scroll(char *str, int *ints)
{
}

/*	kgi_boot_init() is called to setup a console_print() method
**	to display printk()-messages before the KGI manager is running.
**	There are no kernel services (PCI scan, kgi_malloc_*() working
**	when this is called!
*/
ulong kgi_boot_init(ulong kmem_start, ulong kmem_end)
{
#	ifdef CONFIG_VT_CONSOLE
		kgi_boot_need_wrap = (ORIG_X == ORIG_VIDEO_COLS);
		kgi_boot_pos = ORIG_Y*ORIG_VIDEO_COLS + ORIG_X;
		kgi_boot_fb = phys_to_virt(((ORIG_VIDEO_MODE == 7)
				? 0xB0000 : 0xB8000));

#		ifdef CONFIG_KGI_SPLASH

			if (ORIG_VIDEO_ISVGA) {

				show_msg(kgi_boot_fb, ORIG_VIDEO_COLS, 
					ORIG_VIDEO_LINES);
			}
#		endif

		register_console(&kgi_boot_console);
#	endif

	return kmem_start;
}

/*	dpy_i386_init() has to scan for displays the kernel can support.
**	All kernel services are accessible here, as kernel boot is already
**	complete. Later we may allow to link modules directly with the kernel,
**	for now this simply sets up a dumb framebuffer display for every
**	textmode card installed.
**
** !!!	Work over again, we assume text16.80x25.8x14 mode on all displays! 
*/

#define	COLOR_FB	0x000B8000
#define	MONO_FB		0x000B0000
#define	COLOR_CRTC	0x3D4
#define	MONO_CRTC	0x3B4

static inline int detect(kgi_u16_t port)
{
	kgi_u8_t old, read;

	outb_p(0x0F, port++);
	old  = inb_p(port);
	outb_p(0xA5, port);
	read = inb_p(port);
	outb_p(old, port);

	if (read == 0xA5) {

		/*	gotcha, we have to hide the hardware cursor
		*/
		outb_p(0xFF, port--);
		outb_p(0x0E, port++);
		outb_p(0xFF, port);

		return 1;
	}
	return 0;
}

int dpy_i386_init(int display, int max_display)
{
	kgi_display_t *dpy;

	if ((display < max_display) && detect(COLOR_CRTC)) {

		dpy = text16_malloc_display(COLOR_FB, 4 KB, 
			80,25, 80, 720,400, COLOR);
		if (!dpy || kgi_register_display(dpy, display)) {

			KRN_NOTICE("Could not register color display.\n");
		}
		display++;
	}

	if ((display < max_display) && detect(MONO_CRTC)) {

		dpy = text16_malloc_display(MONO_FB, 4 KB,
			80,25, 80, 720,350, MONO);
		if (!dpy || kgi_register_display(dpy, display)) {

			KRN_NOTICE("Could not register mono display.\n");
		}
		display++;
	}

#	ifdef CONFIG_KGI_SPLASH
		if (ORIG_VIDEO_ISVGA) {

			hide_msg(kgi_boot_fb, ORIG_VIDEO_COLS, ORIG_VIDEO_LINES);
		}
#	endif

	unregister_console(&kgi_boot_console);

	return display;
}
