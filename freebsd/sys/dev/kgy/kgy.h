/*-
 * Copyright (C) 2002 Nicholas Souchu
 * 
 * This file is distributed under the terms and conditions of the 
 * MIT/X public license. Please see the file COPYRIGHT.MIT included
 * with this software for details of these terms and conditions.
 * Alternatively you may distribute this file under the terms and
 * conditions of the GNU General Public License. Please see the file 
 * COPYRIGHT.GPL included with this software for details of these terms
 * and conditions.
 */

/*
 * KGI vidsw interface display driver.
 */

#ifndef _kgi_kgy_h
#define _kgi_kgy_h

#define BIT_REVERSE(byte)		\
	((((byte) & 0x01) << 7) |	\
	 (((byte) & 0x02) << 5) |	\
	 (((byte) & 0x04) << 3) |	\
	 (((byte) & 0x08) << 1) |	\
	 (((byte) & 0x10) >> 1) |	\
	 (((byte) & 0x20) >> 3) |	\
	 (((byte) & 0x40) >> 5) |	\
	 (((byte) & 0x80) >> 7))

typedef struct {
	kgi_u_t width;
	kgi_u_t height;
	kgi_u8_t  data[256 * 32];
} dpysw_font_t;

typedef struct dpysw_display_s {
	kgi_display_t dpy;		/* Must remain on top of struct. */

	video_adapter_t *adp;	
	kgi_mode_t mode;

	/* Fonts. */
	dpysw_font_t fonts[4];
	kgi_u8_t *curfont;

	/* Display resources. */
	kgi_mmio_region_t fb;

	kgi_clut_t ilut;
	kgi_marker_t ptr;

	kgi_text16_t text16;
} dpysw_display_t;

typedef struct vidsw_mode_s {
	kgi_dot_port_mode_t dpm;
	video_info_t mode_info;
	video_info_t oldmode_info;
} vidsw_mode_t;

extern dpysw_display_t dpysw_sc;

extern void kgy_splash(video_adapter_t *adp);

extern void dpysw_set_mode(kgi_display_t *dpy, kgi_image_mode_t *img,
			   kgi_u_t images, void *dev_mode);

extern void dpysw_unset_mode(kgi_display_t *dpy, kgi_image_mode_t *img,
			     kgi_u_t images, void *dev_mode);

extern int dpysw_check_mode(kgi_display_t *dpy, kgi_timing_command_t cmd,
			    kgi_image_mode_t *img, kgi_u_t images,
			    void *dev_mode, const kgi_resource_t **r,
			    kgi_u_t rsize);

extern void dpysw_load_font(kgi_text16_t *text16, kgi_u_t page,
			    kgi_u_t fontsize, kgi_u_t fontwidth, kgi_u8_t *data,
			    kgi_u_t ch, kgi_s_t count);

extern void dpysw_save_font(kgi_text16_t *text16, kgi_u_t page,
			    kgi_u_t fontsize, kgi_u_t fontwidth, kgi_u8_t *data,
			    kgi_u_t ch, kgi_s_t count);

extern void dpysw_show_font(kgi_text16_t *text16, kgi_u_t page);

extern void dpysw_put_text16(kgi_text16_t *text16, kgi_u_t offset,
			     const kgi_u16_t *text, kgi_u_t count);

#endif /* _kgi_kgy_h */
