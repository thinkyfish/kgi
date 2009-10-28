/*-
 * Copyright (c) 1999 Michael Smith <msmith@freebsd.org>
 * Copyright (c) 1999 Kazutaka YOKOTA <yokota@freebsd.org>
 * Copyright (c) 1999 Dag-Erling Coïdan Smørgrav
 * Copyright (c) 2003 Nicholas Souchu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * From FreeBSD: src/sys/modules/backgnd/pcx/backgnd_pcx.c,v 1.5 2002/11/11 10:28:44 mux Exp
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_kgirndr.h>
#include <dev/kgc/kgc_backgnd.h>

#define FADE_TIMEOUT	300	/* sec */

static int pcx_start(render_t r);
static int pcx_end(render_t r);
static int pcx_init(char *data, int sdepth);
static int pcx_draw(render_t r, unsigned char *vidmem, kgi_u16_t *pal);

static backgnd_decoder_t pcx_decoder = {
	"backgnd_pcx", pcx_start, pcx_end, pcx_draw, BACKGND_IMAGE,
};

BACKGND_DECODER(backgnd_pcx, pcx_decoder);

static struct {
	int width, height, bpsl;
	int bpp, zlen;
	const u_char *zdata;
	u_char *palette;
} pcx_info;

extern const char daemon_pcx[];

static int 
pcx_start(render_t r)
{
	kgirndr_meta *render;
	kgi_image_mode_t *img = &render->mode.img[0];
	int depth;

	render = kgc_render_meta(r);

	/* Use the nebula daemon by default */
	if (pcx_decoder.data == NULL || pcx_decoder.data_size <= 0) {
		pcx_decoder.data = daemon_pcx;
		pcx_decoder.data_size = 40386;
	}

	if (pcx_init((u_char *)pcx_decoder.data, pcx_decoder.data_size))
		return (ENODEV);

	if (bootverbose) {
		printf("backgnd_pcx: image good:\n"
			"  width = %d\n"
			"  height = %d\n"
			"  depth = %d\n",
			pcx_info.width, pcx_info.height,
			pcx_info.bpp);
	}

	depth = kgi_attr_bits(img->bpfa);
    
	if (bootverbose)
		printf("backgnd_pcx: considering mode:\n"
			"  width = %d\n"
			"  height = %d\n"
			"  depth = %d\n",
			img->size.y, img->size.x, depth);

	if (img->size.x < pcx_info.width || img->size.y < pcx_info.height
			|| depth != pcx_info.bpp)
		return (ENODEV);

	return (0);
}

static int
pcx_end(render_t r)
{

	/* nothing to do */
	return (0);
}

struct pcxheader {
	u_char manufactor;
	u_char version;
	u_char encoding;
	u_char bpp;
	u_short xmin, ymin, xmax, ymax;
	u_short hres, vres;
	u_char colormap[48];
	u_char rsvd;
	u_char nplanes;
	u_short bpsl;
	u_short palinfo;
	u_short hsize, vsize;
};

#define MAXSCANLINE 1024

static int
pcx_init(char *data, int size)
{
	const struct pcxheader *hdr;

	hdr = (const struct pcxheader *)data;

	if (size < 128 + 1 + 1 + 768
		|| hdr->manufactor != 10
		|| hdr->version != 5
		|| hdr->encoding != 1
		|| hdr->nplanes != 1
		|| hdr->bpp != 8
		|| hdr->bpsl > MAXSCANLINE
		|| data[size-769] != 12) {
		printf("backgnd_pcx: invalid PCX image\n");
		return (1);
	}

	pcx_info.width =  hdr->xmax - hdr->xmin + 1;
	pcx_info.height =  hdr->ymax - hdr->ymin + 1;
	pcx_info.bpsl = hdr->bpsl;
	pcx_info.bpp = hdr->bpp;
	pcx_info.zlen = size - (128 + 1 + 768);
	pcx_info.zdata = data + 128;
	pcx_info.palette = data + size - 768;

	return (0);
}

static int
pcx_draw(render_t r, unsigned char *vidmem, kgi_u16_t *pal)
{
	kgirndr_meta *render;
	kgi_image_mode_t *img;
	int swidth, sheight, sbpsl, sdepth;
	int c, i, j, pos, scan, x, y;
	u_char line[MAXSCANLINE];
	kgi_u16_t clut[256*3], *pclut;

	render = kgc_render_meta(r);
	img = &render->mode.img[0];
    
	if (pcx_info.zlen < 1)
		return (1);

	pclut = (pal) ? pal : clut;
    
	/* 
	 * Copy the palette
	 * XXX What to do if palette was not 256 bytes long?
	 * Convert 8bits to 16bits palette. What for?
	 */
	for (i = 0; i < 256 * 3; i += 3) {
		pclut[i] = pcx_info.palette[i] << 8;
		pclut[i + 1] = pcx_info.palette[i + 1] << 8;
		pclut[i + 2] = pcx_info.palette[i + 2] << 8;
	}
	if (pal == NULL && render->ilut->Set)
		/* Set directly the palette with the display resource */
		(render->ilut->Set)(render->ilut, 0, 0, 256, KGI_AM_COLORS, 
			pclut);
      
	swidth = img->size.x;
	sheight = img->size.y;
	sdepth = kgi_attr_bits(img->bpfa);

	/* Add 1 for the case of 15bits mode */
	sbpsl = swidth * ((sdepth + 1) / 8);
    
	bzero(vidmem, sheight*sbpsl);
    
	x = (swidth - pcx_info.width) / 2;
	y = (sheight - pcx_info.height) / 2;
	pos = y * sbpsl + x;
    
	for (scan = i = 0; scan < pcx_info.height; ++scan, ++y, pos += sbpsl) {
		for (j = 0; j < pcx_info.bpsl && i < pcx_info.zlen; ++i) {
			if ((pcx_info.zdata[i] & 0xc0) == 0xc0) {
				c = pcx_info.zdata[i++] & 0x3f;
				if (i >= pcx_info.zlen)
					return (1);
			} else 
				c = 1;

			if (j + c > pcx_info.bpsl)
				return (1);
	
			while (c--)
				line[j++] = pcx_info.zdata[i];
		}
		bcopy(line, vidmem + pos, pcx_info.width);
	}

	return (0);
}
