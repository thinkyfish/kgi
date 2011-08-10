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
 * KGI command definitions
 */

#ifndef _KGI_COMMAND_H_
#define	_KGI_COMMAND_H_

/*
 * null command data. No input, no output.
 */
typedef struct {
} kgic_null_t;

#define	kgic_null_result_t	kgic_null_t
#define	kgic_null_request_t	kgic_null_t

#define KGIC_UNION(type, command)			\
typedef union {						\
	kgic_##type##_##command##_request_t request;	\
	kgic_##type##_##command##_result_t  result;	\
} kgic_##type##_##command##_union_t;

/*
 * mapper commands
 */
typedef struct {
	kgi_ascii_t	client[64];
	kgi_version_t	client_version;
} kgic_mapper_identify_request_t;

typedef struct {
	kgi_ascii_t	mapper[64];
	kgi_version_t	mapper_version;
	kgi_u32_t	resources;
} kgic_mapper_identify_result_t;

KGIC_UNION(mapper, identify);

typedef struct {
	kgi_u32_t	images;
} kgic_mapper_set_images_request_t;

typedef kgic_null_result_t kgic_mapper_set_images_result_t;

KGIC_UNION(mapper, set_images);

typedef kgic_null_request_t kgic_mapper_get_images_request_t;

typedef struct {
	kgi_u32_t	images;
} kgic_mapper_get_images_result_t;

KGIC_UNION(mapper, get_images);

typedef struct {
	kgi_u32_t		image;
	kgi_image_mode_t	mode;
} kgic_mapper_set_image_mode_request_t;

typedef kgic_null_result_t kgic_mapper_set_image_mode_result_t;

KGIC_UNION(mapper, set_image_mode);

typedef struct {
	kgi_u32_t	image;
} kgic_mapper_get_image_mode_request_t;

typedef struct {
	kgi_image_mode_t	mode;
} kgic_mapper_get_image_mode_result_t;

KGIC_UNION(mapper, get_image_mode);

#define __KGIC_RESOURCE_REQUEST_COMMON \
	kgi_u_t image, resource;

/*
 * set image in kgic_mapper_resource_info_request_t to this get mode
 * resource info.  0 to __KGI_MAX_NR_IMAGE_RESOURCES will get per image
 * resource info.
 */
#define	KGIC_MAPPER_NON_IMAGE_RESOURCE   -1

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON
} kgic_mapper_resource_info_request_t;

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON
	kgi_ascii_t		name[64];
	kgi_resource_type_t	type;
	kgi_protection_flags_t	protection;
	union {
		struct {
			kgi_u32_t	access;
			kgi_u32_t	align;
			kgi_size_t	size;
			kgi_size_t	window;
		} mmio;
		struct {
			kgi_u32_t	buffers;
			kgi_size_t	buffer_size;
		} accel;
		struct {
			kgi_size_t		aperture_size;
		} shmem;
	} info;
} kgic_mapper_resource_info_result_t;

KGIC_UNION(mapper, resource_info);

typedef struct {
	kgi_resource_type_t type;
	__KGIC_RESOURCE_REQUEST_COMMON
	union {
		struct {
			kgi_u_t buffers;
			kgi_u_t max_order;
			kgi_u_t min_order;
			kgi_u_t priority;
		} accel;
	} private;
} kgic_mapper_mmap_setup_request_t;

typedef kgic_null_result_t kgic_mapper_mmap_setup_result_t;

KGIC_UNION(mapper, mmap_setup);

typedef struct {
	kgi_u_t device_id;
} kgic_mapper_attach_request_t;

typedef kgic_null_result_t kgic_mapper_attach_result_t;

KGIC_UNION(mapper, attach);

typedef struct {
	kgi_u_t unit;
} kgic_mapper_get_unit_request_t;

typedef struct {
	kgi_u_t unit;
} kgic_mapper_get_unit_result_t;

KGIC_UNION(mapper, get_unit);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
} kgic_resource_request_t;


/*
 * {c,a,i,t}lut commands
 */

typedef kgic_null_result_t kgic_clut_get_info_request_t;

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t tables;
	kgi_u_t entries;
} kgic_clut_get_info_result_t;

KGIC_UNION(clut, get_info);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t	lut;
} kgic_clut_select_request_t;

typedef kgic_null_result_t kgic_clut_select_result_t;

KGIC_UNION(clut, select);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t	lut;
	kgi_u_t	idx;
	kgi_u_t cnt;
	kgi_attribute_mask_t am;
	kgi_u16_t *data;
} kgic_clut_set_request_t;

typedef kgic_null_result_t kgic_clut_set_result_t;

KGIC_UNION(clut, set);

/* alut */
typedef kgic_clut_select_request_t 	kgic_alut_select_request_t;
typedef kgic_null_result_t 		kgic_alut_select_result_t;
typedef kgic_clut_set_request_t 	kgic_alut_set_request_t;
typedef kgic_null_result_t 		kgic_alut_set_result_t;

KGIC_UNION(alut, select);
KGIC_UNION(alut, set);

/* ilut */
typedef kgic_clut_select_request_t 	kgic_ilut_select_request_t;
typedef kgic_null_result_t 		kgic_ilut_select_result_t;
typedef kgic_clut_set_request_t 	kgic_ilut_set_request_t;
typedef kgic_null_result_t 		kgic_ilut_set_result_t;

KGIC_UNION(ilut, select);
KGIC_UNION(ilut, set);

/* tlut */
typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u16_t	lut;		/* image and lut number */
} kgic_tlut_select_request_t;

typedef kgic_null_result_t kgic_tlut_select_result_t;

KGIC_UNION(tlut, select);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u16_t	lut;		/* image and lut number	*/
	kgi_u16_t	idx;		/* first entry to set	*/
	kgi_u16_t	cnt;
	kgi_u16_t	dx;		/* bitmap size		*/
	kgi_u16_t	dy;
	kgi_size_t	size;		/* size of the data buffer */
	kgi_u8_t	*data;
} kgic_tlut_set_request_t;

typedef kgic_null_result_t kgic_tlut_set_result_t;

KGIC_UNION(tlut, set);

/*
 * marker commands
 */
typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
} kgic_marker_info_request_t;

typedef struct {
	kgi_marker_mode_t 	modes;
	kgi_u8_t 		shapes;
	kgi_u8_coord_t 		size;
} kgic_marker_info_result_t;

KGIC_UNION(marker, info);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_marker_mode_t mode;
} kgic_marker_set_mode_request_t;

typedef kgic_null_result_t kgic_marker_set_mode_result_t;

KGIC_UNION(marker, set_mode);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t shape;
} kgic_marker_select_request_t;

typedef kgic_null_result_t kgic_marker_select_result_t;

KGIC_UNION(marker, select);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t		shape;
	kgi_u_t 	hot_x;
	kgi_u_t		hot_y;
	void 		*data;
	kgi_rgb_color_t *color;
} kgic_marker_set_shape_request_t;

typedef kgic_null_result_t kgic_marker_set_shape_result_t;

KGIC_UNION(marker, set_shape);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t x;
	kgi_u_t y;
} kgic_marker_show_request_t;

typedef kgic_null_result_t kgic_marker_show_result_t;

KGIC_UNION(marker, show);

/*
 * text16 commands
 */

typedef kgic_null_request_t kgic_text16_info_request_t;

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_ucoord_t size;
	kgi_ucoord_t virt;
	kgi_ucoord_t cell;
	kgi_ucoord_t font;
} kgic_text16_info_result_t;

KGIC_UNION(text16, info);

typedef struct {
	__KGIC_RESOURCE_REQUEST_COMMON;
	kgi_u_t 	offset;
	kgi_u16_t 	*text;
	kgi_u_t 	cnt;
} kgic_text16_put_text16_request_t;

typedef kgic_null_result_t kgic_text16_put_text16_result_t;

KGIC_UNION(text16, put_text16);

/*
 * display commands
 */


/*
 * KGI command encoding
 */

/* Include ioccom.h for _IOxx definitions */
#include <sys/ioccom.h>

#define KGIC_IO(type, command, callback, code) \
	KGIC_##type##_##command = _IO(KGIC_##type##_COMMAND >> 8, code)

#define KGIC_IOR(type, command, callback, code) \
	KGIC_##type##_##command = _IOR(KGIC_##type##_COMMAND >> 8, \
		code, kgic_##callback##_result_t)

#define KGIC_IOW(type, command, callback, code) \
	KGIC_##type##_##command = _IOW(KGIC_##type##_COMMAND >> 8, \
		code, kgic_##callback##_request_t)

#define KGIC_IOWR(type, command, callback, code) \
	KGIC_##type##_##command = _IOWR(KGIC_##type##_COMMAND >> 8, \
		code, kgic_##callback##_union_t)


typedef enum {
	KGIC_MAPPER_COMMAND	= 0x00000000,	/* ext. mapper commands	*/
	KGIC_IOWR(MAPPER, IDENTIFY, 		mapper_identify, 	0x0001),
	KGIC_IOW (MAPPER, SET_IMAGES, 		mapper_set_images,	0x0002),
	KGIC_IOR (MAPPER, GET_IMAGES,		mapper_get_images,	0x0003),
	KGIC_IOW (MAPPER, SET_IMAGE_MODE,	mapper_set_image_mode,	0x0004),
	KGIC_IOWR(MAPPER, GET_IMAGE_MODE,	mapper_get_image_mode,	0x0005),
	KGIC_IO  (MAPPER, MODE_CHECK,		null,			0x0006),
	KGIC_IO  (MAPPER, MODE_SET,		null,			0x0007),
	KGIC_IO  (MAPPER, MODE_UNSET,		null,			0x0008),
	KGIC_IO  (MAPPER, MODE_DONE,		null,			0x0009),
	KGIC_IOWR(MAPPER, RESOURCE_INFO,	mapper_resource_info,	0x000A),
	KGIC_IOW (MAPPER, MMAP_SETUP,		mapper_mmap_setup,	0x000B),
	KGIC_IOW (MAPPER, ATTACH,		mapper_attach,		0x000C),
	KGIC_IOWR(MAPPER, GET_UNIT,		mapper_get_unit,	0x000D),
	KGIC_RESOURCE_COMMAND   = 0x00000100,   /* resource commands */
	/* commands for KGI_RT_{ILUT,ALUT}_CONTROL */
	KGIC_IOR (RESOURCE, CLUT_GET_INFO,	clut_get_info,		0x0001),
	KGIC_IOW (RESOURCE, CLUT_SELECT,	clut_select,		0x0002),
	KGIC_IOW (RESOURCE, CLUT_SET,		clut_set,		0x0003),
	/* commands for KGI_RT_TLUT_CONTROL */
	KGIC_IOW (RESOURCE, TLUT_SELECT,	tlut_select,		0x0020),
	KGIC_IOW (RESOURCE, TLUT_SET,		tlut_set,		0x0021),
	/* commands from KGI_RT_{CURSOR,POINTER}_CONTROL */
	KGIC_IOW (RESOURCE, MARKER_SET_MODE,	marker_set_mode,	0x0030),
	KGIC_IOW (RESOURCE, MARKER_SELECT,	marker_select,		0x0031),
	KGIC_IOW (RESOURCE, MARKER_SET_SHAPE,	marker_set_shape,	0x0032),
	KGIC_IOW (RESOURCE, MARKER_SHOW,	marker_show,		0x0033),
	KGIC_IO  (RESOURCE, MARKER_HIDE,	null,			0x0034),
	KGIC_IO  (RESOURCE, MARKER_UNDO,	null,			0x0035),
	KGIC_IOWR(RESOURCE, MARKER_INFO,	marker_info,		0x0036),
	/* commands for KGI_RT_TEXT16_CONTROL */
	KGIC_IOW (RESOURCE, TEXT16_PUT_TEXT16,	text16_put_text16,	0x0040),
	KGIC_IOR (RESOURCE, TEXT16_INFO,	text16_info,		0x0041),
	KGIC_MODE_COMMAND	= 0x00008000,	/* mode commands	*/
	KGIC_DISPLAY_COMMAND	= 0x0000C000,	/* display commands	*/
	KGIC_TYPE_MASK		= 0x0000FF00,
	KGIC_NR_MASK		= 0x000000FF,
	KGIC_COMMAND_MASK	= KGIC_TYPE_MASK | KGIC_NR_MASK
} kgi_command_t;

#define	KGIC_COMMAND(cmd)	((cmd) & KGIC_COMMAND_MASK)
#define	KGIC_SIZE(cmd)		(IOCPARM_LEN(cmd))
#define	KGIC_READ(cmd)		((cmd) & IOC_OUT)
#define	KGIC_WRITE(cmd)		((cmd) & IOC_IN)

#endif	/* !_KGI_COMMAND_H_ */
