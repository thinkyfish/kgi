/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2002-2004 Nicholas Souchu
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
 * KGI manager OS kernel independent stuff
 */

#ifndef _kgi_kgimod_h
#define _kgi_kgimod_h

/* 
 * Attributes
 * NOTE	Attribute ordering __must__ not be changed! 
 */
typedef enum {
	KGI_A_COLOR1,		/* intensity of color channel 1	*/
	KGI_A_COLOR2,		/* intensity of color channel 2 */
	KGI_A_COLOR3,		/* intensity of color channel 3	*/
	KGI_A_COLOR_INDEX,	/* color index value		*/
	KGI_A_ALPHA,		/* alpha value			*/
	KGI_A_PRIVATE,		/* hardware or driver private	*/
	KGI_A_APPLICATION,	/* store what you want here	*/

	KGI_A_STENCIL,		/* stencil buffer		*/
	KGI_A_Z,		/* z-value			*/

	KGI_A_FOREGROUND_INDEX,	/* foreground color index	*/
	KGI_A_TEXTURE_INDEX,	/* texture index		*/
	KGI_A_BLINK,		/* blink bit/frequency		*/

	KGI_A_LAST,
} kgi_attribute_t;

#define	__KGI_AM(x)	KGI_AM_##x = (1 << KGI_A_##x)
typedef enum {
	__KGI_AM(PRIVATE),
	__KGI_AM(APPLICATION),
	__KGI_AM(STENCIL),
	__KGI_AM(Z),
	__KGI_AM(COLOR_INDEX),
	__KGI_AM(ALPHA),
	__KGI_AM(COLOR1),
	__KGI_AM(COLOR2),
	__KGI_AM(COLOR3),
	__KGI_AM(FOREGROUND_INDEX),
	__KGI_AM(TEXTURE_INDEX),
	__KGI_AM(BLINK),

	KGI_AM_COLORS = KGI_AM_COLOR1 | KGI_AM_COLOR2 | KGI_AM_COLOR3,
	KGI_AM_ALL = (1 << KGI_A_LAST) - 1
} kgi_attribute_mask_t;
#undef __KGI_AM
#define	KGI_ATTRIBUTE_MASK(attr)	(1 << (attr))

#define	KGI_AM_TEXT	(kgi_attribute_mask_t)(KGI_AM_COLOR_INDEX | KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX)
#define	KGI_AM_I	(kgi_attribute_mask_t)(KGI_AM_COLOR_INDEX)
#define	KGI_AM_RGB	(kgi_attribute_mask_t)(KGI_AM_COLORS)
#define	KGI_AM_RGBI	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_COLOR_INDEX)
#define	KGI_AM_RGBA	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_ALPHA)
#define	KGI_AM_RGBX	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_APPLICATION)
#define	KGI_AM_RGBP	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_PRIVATE)

/* attribute strings */
#define	KGI_AS_8888	((const kgi_u8_t[]) { 8,8,8,8,0 })
#define	KGI_AS_4444	((const kgi_u8_t[]) { 4,4,4,4,0 })
#define	KGI_AS_5551	((const kgi_u8_t[]) { 5,5,5,1,0 })
#define	KGI_AS_2321	((const kgi_u8_t[]) { 2,3,2,1,0 })
#define	KGI_AS_4642	((const kgi_u8_t[]) { 4,6,4,3,0 })
#define	KGI_AS_AAA2	((const kgi_u8_t[]) { 10,10,10,2,0 })
#define	KGI_AS_332	((const kgi_u8_t[]) { 3,3,2,0 })
#define	KGI_AS_565	((const kgi_u8_t[]) { 5,6,5,0 })
#define	KGI_AS_448	((const kgi_u8_t[]) { 4,4,8,0 })
#define	KGI_AS_664	((const kgi_u8_t[]) { 6,6,4,0 })
#define	KGI_AS_888	((const kgi_u8_t[]) { 8,8,8,0 })
#define	KGI_AS_ACA	((const kgi_u8_t[]) { 10,12,10,0 })
#define	KGI_AS_88	((const kgi_u8_t[]) { 8,8,0 })
#define	KGI_AS_8	((const kgi_u8_t[]) { 8,0 })
#define	KGI_AS_4	((const kgi_u8_t[]) { 4,0 })
#define	KGI_AS_2	((const kgi_u8_t[]) { 2,0 })
#define	KGI_AS_1	((const kgi_u8_t[]) { 1,0 })

/*
 * timing/modes
 */
typedef enum {
	KGI_TC_PROPOSE,			/* propose a timing		*/
	KGI_TC_LOWER,			/* only lower clock rate	*/
	KGI_TC_RAISE,			/* only raise clock rate	*/
	KGI_TC_CHECK,			/* don't modify but check	*/
	KGI_TC_READY			/* don't care.			*/
} kgi_timing_command_t;

/*
 * dot ports are used to connect dot stream sources (image serializers),
 * dot stream converters (DACs, ...) and dot stream sinks (monitors,...)
 */
typedef enum {
	/* color space */
	KGI_DPF_CS_LIN_RGB	= 0x00000000,	/* linear rgb space	*/
	KGI_DPF_CS_LIN_BGR	= 0x00000001,	/* linear bgr space	*/
	KGI_DPF_CS_LIN_YUV	= 0x00000002,	/* linear yuv space	*/
	KGI_DPF_CS_MASK		= 0x000000FF,

	/* changeable (after mode set) properties */
	KGI_DPF_CH_ORIGIN	= 0x00000100,	/* image orgin		*/
	KGI_DPF_CH_SIZE		= 0x00000200,	/* image size		*/
	KGI_DPF_CH_ILUT		= 0x00000400,	/* index->attr look up	*/
	KGI_DPF_CH_ALUT		= 0x00000800,	/* attr->attr look up	*/
	KGI_DPF_CH_TLUT		= 0x00001000,	/* tindex->pixel texture*/
	KGI_DPF_CH_MASK		= 0x0000FF00,

	/* dot transfer protocol */
	KGI_DPF_TP_LRTB_I0	= 0x00000000,	/* l>r, t>b, noninterl.	*/
	KGI_DPF_TP_LRTB_I1	= 0x00010000,	/* l>r, t>b, interl.	*/
	KGI_DPF_TP_2XCLOCK	= 0x00080000,	/* load twice per cycle	*/
	KGI_DPF_TP_MASK		= 0x000F0000,
	/* ??? stereo handling ? */

	KGI_DPF_MASK		= 0x000FFFFF
} kgi_dot_port_flags_t;

typedef struct {
	kgi_dot_port_flags_t	flags;	/* flags		*/
	kgi_ucoord_t		dots;	/* image size in dots	*/
	kgi_u_t			dclk;	/* (max) dot clock	*/
	kgi_ratio_t		lclk;	/* load clock per dclk	*/
	kgi_ratio_t		rclk;	/* ref clock per dclk	*/
	kgi_attribute_mask_t	dam;	/* dot attr mask */
	const kgi_u8_t		*bpda;	/* bits per dot attribute */
} kgi_dot_port_mode_t;

/*
 * A dot stream converter (DSC) reads data from it's input dot port(s),
 * performs a conversion on it and outputs the result on the output port.
 */
typedef struct {
	kgi_dot_port_mode_t	*out;
	kgi_u_t			inputs;
	kgi_dot_port_mode_t	in[1];
} kgi_dsc_mode_t;

/* images */
typedef enum {
	KGI_IF_ORIGIN	= 0x00000001,	/* origin can be changed	*/
	KGI_IF_VIRTUAL	= 0x00000002,	/* virtual size can be changed	*/
	KGI_IF_VISIBLE	= 0x00000004,	/* visible size can be changed	*/
	KGI_IF_TILE_X	= 0x00000008,	/* can do virtual x tiling	*/
	KGI_IF_TILE_Y	= 0x00000010,	/* can do virtual y tiling	*/
	KGI_IF_ILUT	= 0x00000020,	/* can do index -> attribute	*/
	KGI_IF_ALUT	= 0x00000040, 	/* can do attribute->attribute	*/
	KGI_IF_TLUT	= 0x00000080,	/* can do pixel texture look-up	*/
	KGI_IF_STEREO	= 0x00000100,	/* stereo image			*/
	KGI_IF_TEXT16	= 0x00000200,	/* can do text16 output		*/

	KGI_IF_ALL	= 0x000003FF	/* all flags known		*/
} kgi_image_flags_t;

typedef struct {
	kgi_dot_port_mode_t *out;
	kgi_image_flags_t flags;

	kgi_ucoord_t	virt;
	kgi_ucoord_t	size;

	kgi_u8_t	frames;
	kgi_u8_t	tluts;
	kgi_u8_t	iluts;
	kgi_u8_t	aluts;
	kgi_attribute_mask_t	ilutm;
	kgi_attribute_mask_t	alutm;

	kgi_attribute_mask_t	fam, cam; /* frame, common attribute mask     */
	kgi_u8_t	bpfa[__KGI_MAX_NR_ATTRIBUTES];/* bits per frame attr  */
	kgi_u8_t	bpca[__KGI_MAX_NR_ATTRIBUTES];/* bits per common attr */

	kgi_resource_t	*resource[__KGI_MAX_NR_IMAGE_RESOURCES];
} kgi_image_mode_t;

typedef enum {
	KGI_MF_ALLOCATED		= 0x00000001,	/* has been allocated */
	KGI_MF_BOOT			= 0x00000002	/* mode for boot console */
} kgi_mode_flags_t;

#define	KGI_MODE_REVISION	0x00010001
typedef struct {
	kgi_u_t			revision;  /* data structure revision		*/
	void			*dev_mode; /* device dependent mode		*/
	kgi_mode_flags_t	flags;     /* mode flags                	*/

	const kgi_resource_t	*resource[__KGI_MAX_NR_RESOURCES];

	kgi_u_t			images;	   /* number of images		*/
	kgi_image_mode_t	img[1];	   /* image(s)			*/
} kgi_mode_t;

#endif /* _kgi_kgimod_h */
