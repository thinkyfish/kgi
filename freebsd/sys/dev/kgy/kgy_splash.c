/*-
 * Copyright (c) 2002-2005 Nicholas Souchu
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
 * KGI textmode colour splash screen.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define KGI_DBG_LEVEL 2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/vgareg.h>

#define	KGI_SYS_NEED_IO
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgy/kgy.h>

#if defined(__i386__) && !defined(KGI_NOSPLASH)

#define outb_p(data,port) outb(port, data)
#define inb_p(port) inb(port)

/*
 * This a Quick&Dirty(TM) Hack to be able to slowdown the printk 
 * to follow the early kernel boot messages in case there are 
 * kernel-Ooops's before we have scrollback.
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

static void 
set_col(void)
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

	if (inb_p(0x3DA) & 8) 
		while (inb_p(0x3DA) & 8);

	while (!(inb_p(0x3DA) & 8));
}

static void 
show_msg(kgi_u16_t *fb, kgi_u_t sizex, kgi_u_t sizey)
{
	int line = 0, firstrow = (sizex - 80) / 2, i;

	fb += ((sizey - 25) / 2) * sizex;

	curr_col[7] = def_col[7];
	set_col();

	while (line < 25) {
		int row;
		kgi_u16_t *p = fb + firstrow;

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
			curr_col[reg].r = msg_col[reg].r * i / 100;
			curr_col[reg].g = msg_col[reg].g * i / 100;
			curr_col[reg].b = msg_col[reg].b * i / 100;
		}

		curr_col[7].r = 
			(msg_col[7].r - def_col[7].r) * i / 100 + def_col[7].r;
		curr_col[7].g = 
			(msg_col[7].g - def_col[7].g) * i / 100 + def_col[7].g;
		curr_col[7].b = 
			(msg_col[7].b - def_col[7].b) * i / 100 + def_col[7].b;

		set_col();
	}

	for (i = 0; i < 100; i++) {
		curr_col[6].r = 
			(63 - msg_col[6].r) * i / 100 + msg_col[6].r;
		curr_col[6].g = 
			(63 - msg_col[6].g) * i / 100 + msg_col[6].g;
		curr_col[6].b = 
			(0 - msg_col[6].b) * i / 100 + msg_col[6].b;
		set_col();
	}

	for (i = 0; i < 100; i++) {
		curr_col[6].r = (msg_col[6].r - 63) * i / 100 + 63;
		curr_col[6].g = (msg_col[6].g - 63) * i / 100 + 63;
		curr_col[6].b = (msg_col[6].b -  0) * i / 100 +  0;
		set_col();
	}
}

static void 
hide_msg(kgi_u16_t *fb, kgi_u_t sizex, kgi_u_t sizey)
{
	int i;

	for (i = 100; i >= 0; i--) {
		int reg;

		for (reg = 0; reg < 7; reg++) {
			curr_col[reg].r = msg_col[reg].r * i / 100;
			curr_col[reg].g = msg_col[reg].g * i / 100;
			curr_col[reg].b = msg_col[reg].b * i / 100;
		}

		curr_col[7].r = 
			(msg_col[7].r-def_col[7].r) * i / 100 + def_col[7].r;
		curr_col[7].g = 
			(msg_col[7].g-def_col[7].g) * i / 100 + def_col[7].g;
		curr_col[7].b = 
			(msg_col[7].b-def_col[7].b) * i / 100 + def_col[7].b;

		set_col();
	}

	i = sizex * sizey;

	while (i--) {
		*fb = (*fb & 0x00FF) | 0x0700;
		fb++;
	}

	for (i = 0; i < 8; i++) 
		curr_col[i] = def_col[i];

	set_col();
}

#endif /* __i386__ && !KGI_NOSPLASH */

#define ORIG_VIDEO_COLS (adp->va_info.vi_width)
#define ORIG_VIDEO_LINES (adp->va_info.vi_height)

void 
kgy_splash(video_adapter_t *adp)
{

#if defined(__i386__) && !defined(KGI_NOSPLASH)

	show_msg((kgi_u16_t *)adp->va_window, ORIG_VIDEO_COLS, 
		ORIG_VIDEO_LINES);
	kgi_udelay(1000000);

	hide_msg((kgi_u16_t *)adp->va_window, ORIG_VIDEO_COLS, 
		ORIG_VIDEO_LINES);

#endif /* __i386__ && !KGI_NOSPLASH */

	return;
}
