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

#ifndef _kgi_kgidpy_h
#define _kgi_kgidpy_h

#ifdef _KERNEL

/*
 * Displays.
 */

#define	KGI_DISPLAY_REVISION	0x00010000

typedef struct kgi_display_s kgi_display_t;

typedef void kgi_display_refcount_fn(kgi_display_t *);

typedef kgi_error_t kgi_display_check_mode_fn(kgi_display_t *dpy,
		kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images,
		void *dev_mode, const kgi_resource_t **r, kgi_u_t rsize);

typedef	void kgi_display_set_mode_fn(kgi_display_t *dpy,
		kgi_image_mode_t *img, kgi_u_t images, void *dev_mode);

typedef kgi_error_t kgi_display_command_fn(kgi_display_t *dpy, kgi_u_t cmd, 
		void *data);

struct kgi_display_s {
	kgi_u_t		revision;	/* KGI/driver revision 	*/
#define KGI_MAX_VENDOR_STRING	64
	kgi_ascii_t	vendor[KGI_MAX_VENDOR_STRING]; /* manufacturer	  */
	kgi_ascii_t	model[KGI_MAX_VENDOR_STRING];  /* model/trademark */
	kgi_u32_t	flags;		/* special capabilities	*/
	kgi_u_t		mode_size;	/* private mode size	*/
	kgi_mode_t	*mode;	/* currently set mode			*/
	kgi_u_t	id;		/* display number, init to -1		*/
	kgi_u_t	graphic;	/* non-console devices attached		*/
	struct kgi_display_s *prev; /* previous driver 		*/
	kgi_display_refcount_fn		*IncRefcount;
	kgi_display_refcount_fn		*DecRefcount;
	kgi_display_check_mode_fn	*CheckMode;
	kgi_display_set_mode_fn		*SetMode;
	kgi_display_set_mode_fn		*UnsetMode;
	kgi_display_command_fn		*Command;
	struct kgi_device_s	*focus;	/* current focus	*/
};

/*
 * KGI events
 */

#define KGI_EVENT_NOTICE_NEW_DISPLAY   0x00000001

typedef struct {
	kgi_u_t		command;
} kgi_event_notice_t;

typedef union {
	kgi_event_notice_t	notice;
} kgi_event_t;

/*
 * devices
 * DF_CONSOLE is not text mode anymore but may be graphic. This is just a
 * question of map. DF_TEXT16 shall be used to specify a text console.
 */
typedef enum {
	KGI_DF_FOCUSED			= 0x00000001,
	KGI_DF_CONSOLE			= 0x00000002,
	KGI_DF_CLONED			= 0x00000004,
	KGI_DF_TEXT16			= 0x00000008
} kgi_device_flags_t;

typedef void kgi_device_map_device_fn(struct kgi_device_s *);
typedef kgi_s_t kgi_device_unmap_device_fn(struct kgi_device_s *);
typedef void kgi_device_handle_event_fn(struct kgi_device_s *, kgi_event_t *);


typedef struct kgi_device_s {
	kgi_u_t			id;	/* device number 		*/
	kgi_u_t			dpy_id;	/* display number		*/
	kgi_device_flags_t	flags;	/* device flags			*/
	kgi_device_map_device_fn	*MapDevice;
	kgi_device_unmap_device_fn	*UnmapDevice;
	kgi_device_handle_event_fn	*HandleEvent;
	kgi_mode_t	*mode;	/* currently set mode			*/
	kgi_private_t	priv;	/* device private state			*/
} kgi_device_t;

extern kgi_u_t kgi_attr_bits(const kgi_u8_t *bpa);

extern void    kgi_map_device(kgi_u_t dev_id);
extern kgi_s_t kgi_unmap_device(kgi_u_t dev_id);

extern kgi_s_t kgi_register_device(kgi_device_t *dev, kgi_u_t idx);
extern void    kgi_unregister_device(kgi_device_t *dev);
extern kgi_error_t kgi_check_mode(kgi_u_t dev_id, kgi_mode_t *m);

extern kgi_s_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id);
extern void    kgi_unregister_display(kgi_display_t *dpy);
extern kgi_s_t kgi_display_registered(kgi_u_t id);

extern kgi_device_t *kgi_current_focus(kgi_u_t dpy_id);
extern kgi_u_t kgi_current_devid(kgi_u_t dpy_id);
extern void kgidev_show_gadgets(kgi_device_t *dev);
extern void kgidev_undo_gadgets(kgi_device_t *dev);

extern kgi_error_t kgidev_display_command(kgi_device_t *dev, kgi_u_t cmd, 
		void *data);

#endif /* _KERNEL */

#endif /* _kgi_kgidpy_h */
