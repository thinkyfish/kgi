/* ----------------------------------------------------------------------------
**	KGI command definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: cmd.h,v $
*/
#ifndef _kgi_command_h
#define	_kgi_command_h

#if 0
typedef struct
{
	kgi_u16_t	img, lut;	/* image and lut number		*/
	kgi_u16_t	idx, cnt;	/* start/number entries to set	*/
	kgi_attribute_mask_t am;	/* attribute(s) to set		*/
	kgi_u16_t	*data;		/* attribute values		*/

} kgic_alut_entries_t;

#define	kgic_ilut_entries_t	kgic_alut_entries_t

typedef struct
{
	kgi_u16_t	img, lut;	/* image and lut number	*/
	kgi_u16_t	idx, cnt;	/* first entry to set	*/
	kgi_u16_t	dx, dy;		/* bitmap size		*/
	kgi_u8_t	*data;

} kgic_tlut_entries_t;


		KGIC_ILUT_SET_ENTRIES,	/* kgic_ilut_entries_header + values */
		KGIC_ILUT_GET_ENTRIES,	/* kgic_ilut_entries_header + values */

		KGIC_ALUT_SET_ENTRIES,	/* kgic_alut_entries_header + values */
		KGIC_ALUT_GET_ENTRIES,	/* kgic_alut_entries_header + values */

		KGIC_TLUT_SET_ENTRIES,	/* kgic_tlut_entries */
#endif

/* ----------------------------------------------------------------------------
**	null command data. No input, no output.
** ----------------------------------------------------------------------------
*/
typedef struct
{
} kgic_null_t;

#define	kgic_null_result_t	kgic_null_t
#define	kgic_null_request_t	kgic_null_t

/* ----------------------------------------------------------------------------
**	mapper commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	kgi_ascii_t	client[64];
	kgi_version_t	client_version;

} kgic_mapper_identify_request_t;

typedef struct
{
	kgi_ascii_t	mapper[64];
	kgi_version_t	mapper_version;
	kgi_u32_t	resources;

} kgic_mapper_identify_result_t;



typedef struct
{
	kgi_u32_t	images;

} kgic_mapper_set_images_request_t;

typedef struct
{
} kgic_mapper_set_images_result_t;



typedef struct
{
} kgic_mapper_get_images_request_t;

typedef struct
{
	kgi_u32_t	images;

} kgic_mapper_get_images_result_t;



typedef struct
{
	kgi_u32_t		image;
	kgi_image_mode_t	mode;

} kgic_mapper_set_image_mode_request_t;

typedef struct
{
} kgic_mapper_set_image_mode_result_t;



typedef struct
{
	kgi_u32_t	image;

} kgic_mapper_get_image_mode_request_t;

typedef struct
{
	kgi_image_mode_t	mode;

} kgic_mapper_get_image_mode_result_t;


typedef struct
{
	kgi_u32_t		resource;

} kgic_mapper_resource_info_request_t;

typedef struct
{
	kgi_u32_t		resource;
	kgi_ascii_t		name[64];
	kgi_resource_type_t	type;
	kgi_protection_flags_t	protection;

	union {
		struct {
			kgi_u32_t	access;
			kgi_u32_t	align;
			kgi_size_t	size;
			kgi_size_t	window;

		}		mmio;

		struct {
			kgi_u32_t	buffers;
			kgi_size_t	buffer_size;

		}		accel;

		struct {
			kgi_size_t		aperture_size;

		}	shmem;

	}			info;

} kgic_mapper_resource_info_result_t;

/* ----------------------------------------------------------------------------
**	mode commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	kgi_display_t	*dpy;		/* display this mode is for	*/
	void		*dev_mode;	/* device specific mode data	*/
	kgi_u_t		images;		/* number of images in img[]	*/
	kgi_image_mode_t *img;		/* array of images		*/

} kgic_mode_context_t;

typedef
kgi_error_t kgic_mode_command_fn(kgic_mode_context_t *ctx,
		kgi_u_t cmd, void *in_buffer,
		void **out_buffer, kgi_size_t *out_size);

/*
**	text rendering API
*/
#define	KGIC_MODE_TEXT16CONTEXT_REVISION	0x0100

typedef struct kgic_mode_text16context_s kgic_mode_text16context_t;

typedef void kgic_mode_marker_show_fn(kgic_mode_text16context_t *ctx,
		kgi_u_t x, kgi_u_t y);
typedef	void kgic_mode_marker_hide_fn(kgic_mode_text16context_t *ctx);
typedef	void kgic_mode_put_text_fn(kgic_mode_text16context_t *ctx, 
		kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count);
typedef void kgic_mode_set_table_fn(kgic_mode_text16context_t *ctx,
		kgi_u_t table, kgi_u_t index, kgi_u_t count, void *data);

struct kgic_mode_text16context_s
{
	kgi_u_t		revision;	/* text (A)PI revision		*/
	void		*meta_object;	/* device specific handle	*/
	void		*meta_mode;	/* device specific mode data	*/

	kgi_ucoord_t	size;		/* visible text cells		*/
	kgi_ucoord_t	virt;		/* virtual text cells		*/
	kgi_ucoord_t	cell;		/* dots per cell		*/
	kgi_ucoord_t	font;		/* dots per font cell		*/

	kgic_mode_marker_show_fn	*CursorShow;
	kgic_mode_marker_hide_fn	*CursorHide;
	kgic_mode_marker_hide_fn	*CursorUndo;

	kgic_mode_marker_show_fn	*PointerShow;
	kgic_mode_marker_hide_fn	*PointerHide;
	kgic_mode_marker_hide_fn	*PointerUndo;

	kgic_mode_put_text_fn		*PutText16;
	kgic_mode_set_table_fn		*SetCLUT;
	kgic_mode_set_table_fn		*SetTLUT;
};


typedef struct
{
	kgi_u_t		revision;	/* 'optimal' (A)PI revision	*/
	kgi_u_t		image;		/* image for text output	*/

} kgic_mode_text16context_request_t;

typedef kgic_mode_text16context_t kgic_mode_text16context_result_t;



/* ----------------------------------------------------------------------------
**	display commands
** ----------------------------------------------------------------------------
*/


/* ----------------------------------------------------------------------------
**	KGI command encoding
** ----------------------------------------------------------------------------
*/
#define	KGIC(command, type, code)					\
	KGIC_##command =						\
		(sizeof(kgic_##type##_request_t) << KGIC_SIZE_SHIFT) |	\
		(sizeof(kgic_##type##_request_t) ? KGIC_RD : 0) |	\
		(sizeof(kgic_##type##_result_t)  ? KGIC_WR : 0) |	\
		(code)

typedef enum
{
	KGIC_RD			= 0x40000000,
	KGIC_WR			= 0x80000000,
	KGIC_SIZE_MASK		= 0x3FFF0000,
	KGIC_SIZE_SHIFT		= 16,

	KGIC_MAPPER_COMMAND	= 0x00000000,	/* ext. mapper commands	*/
		KGIC(MAPPER_IDENTIFY,		mapper_identify,	0x0001),
		KGIC(MAPPER_SET_IMAGES,		mapper_set_images,	0x0002),
		KGIC(MAPPER_GET_IMAGES,		mapper_get_images,	0x0003),
		KGIC(MAPPER_SET_IMAGE_MODE,	mapper_set_image_mode,	0x0004),
		KGIC(MAPPER_GET_IMAGE_MODE,	mapper_get_image_mode,	0x0005),
		KGIC(MAPPER_MODE_CHECK,		null,			0x0006),
		KGIC(MAPPER_MODE_SET,		null,			0x0007),
		KGIC(MAPPER_MODE_UNSET,		null,			0x0008),
		KGIC(MAPPER_MODE_DONE,		null,			0x0009),
		KGIC(MAPPER_RESOURCE_INFO,	mapper_resource_info,	0x000A),

	KGIC_MODE_COMMAND	= 0x00008000,	/* mode commands	*/
		KGIC(MODE_TEXT16CONTEXT,	mode_text16context,	0x8001),

	KGIC_DISPLAY_COMMAND	= 0x0000C000,	/* display commands	*/

	KGIC_TYPE_MASK		= 0x0000FF00,
	KGIC_NR_MASK		= 0x000000FF,
	KGIC_COMMAND_MASK	= KGIC_TYPE_MASK | KGIC_NR_MASK

} kgi_command_t;

#define	KGIC_COMMAND(cmd)	((cmd) & KGIC_COMMAND_MASK)
#define	KGIC_SIZE(cmd)		(((cmd) & KGIC_SIZE_MASK) >> KGIC_SIZE_SHIFT)
#define	KGIC_READ(cmd)		((cmd) & KGIC_RD)
#define	KGIC_WRITE(cmd)		((cmd) & KGIC_WR)

#endif	/* #ifndef _kgi_command_h */
