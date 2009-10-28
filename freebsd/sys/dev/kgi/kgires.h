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
#ifndef _kgi_kgires_h
#define _kgi_kgires_h

typedef enum {
	/*  may be mapped by applications (others) */ 
	KGI_PF_APP_R		= 0x00000001,	/* read access allowed	*/
	KGI_PF_APP_W		= 0x00000002,	/* write access allowed	*/
	KGI_PF_APP_S		= 0x00000004,	/* may be shared	*/
	KGI_PF_APP_RWS		= 0x00000007,	/* read, write, shared	*/
	KGI_PF_APP_WP		= 0x00000008,	/* write protected	*/
	KGI_PF_APP_RP		= 0x00000010,	/* read protected	*/
	KGI_PF_APP		= 0x0000001F,	/* application flags	*/

	/* may be mapped by trusted applications (group) */
	KGI_PF_LIB_R		= 0x00000100,	/* read access allowed	*/
	KGI_PF_LIB_W		= 0x00000200,	/* write access allowed	*/
	KGI_PF_LIB_S		= 0x00000400,	/* may be shared	*/
	KGI_PF_LIB_RWS		= 0x00000700,	/* read, write, shared	*/
	KGI_PF_LIB_WP		= 0x00000800,	/* write protected	*/
	KGI_PF_LIB_RP		= 0x00001000,	/* read protected	*/
	KGI_PF_LIB		= 0x00001F00,	/* library flags	*/

	/* may be mapped only by session leader (owner) */
	KGI_PF_DRV_R		= 0x00010000,	/* read access allowed	*/
	KGI_PF_DRV_W		= 0x00020000,	/* write access allowed	*/
	KGI_PF_DRV_S		= 0x00040000,	/* may be shared	*/
	KGI_PF_DRV_RWS		= 0x00070000,	/* read, write, shared	*/
	KGI_PF_DRV_WP		= 0x00080000,	/* write protected	*/
	KGI_PF_DRV_RP		= 0x00100000,	/* read protected	*/
	KGI_PF_DRV		= 0x001F0000,	/* driver flags		*/

	/* common (global) attributes */
	KGI_PF_WRITE_ORDER	= 0x01000000,	/* write order matters	*/
	KGI_PF_READ_ORDER	= 0x02000000,	/* read order matters	*/
	KGI_PF_WRITE_CACHING	= 0x04000000,	/* allowed if set	*/
	KGI_PF_READ_CACHING	= 0x08000000	/* allowed if set	*/
} kgi_protection_flags_t;

typedef enum {
	KGI_RT_MMIO	= 0x00000000,
		KGI_RT_MMIO_FRAME_BUFFER,	/* linear frame buffer	*/
		KGI_RT_MMIO_LOCAL_BUFFER,	/* linear local buffer	*/
		KGI_RT_MMIO_PRIVATE,		/* private MMIO region	*/

	KGI_RT_SHMEM	= 0x20000000,
		KGI_RT_SHARED_MEMORY,

	KGI_RT_ACCEL	= 0x40000000,
		KGI_RT_ACCELERATOR,

	KGI_RT_COMMAND	= 0x60000000,
		KGI_RT_CURSOR_CONTROL,
		KGI_RT_POINTER_CONTROL,
		KGI_RT_ILUT_CONTROL,
		KGI_RT_ALUT_CONTROL,
		KGI_RT_TLUT_CONTROL,
		KGI_RT_TEXT16_CONTROL,

	KGI_RT_MASK	= 0x70000000,	/* WARNING: offset encoding depends! */

	KGI_RT_LAST
} kgi_resource_type_t;

typedef enum {
	KGI_MM_TEXT16	= 0x00000001,
	KGI_MM_WINDOWS	= 0x00000002,
	KGI_MM_X11	= 0x00000004,
	KGI_MM_3COLOR	= 0x00000008
} kgi_marker_mode_t;

#ifdef _KERNEL

typedef struct {
	kgi_size_t	size;		/* aperture size		*/
	kgi_bus_addr_t	bus;		/* bus address			*/
	kgi_phys_addr_t	phys;		/* physical address		*/
	kgi_virt_addr_t	virt;		/* virtual address		*/
} kgi_aperture_t;

#define	__KGI_RESOURCE	\
	void		*meta;		/* meta language object		*/\
	void		*meta_io;	/* meta language I/O context	*/\
	kgi_resource_type_t	type;	/* type ID			*/\
	kgi_protection_flags_t	prot;	/* protection info		*/\
	const kgi_ascii_t	*name;	/* name/identifier		*/

struct kgi_resource_s {
	__KGI_RESOURCE
};

/* MMIO regions */

typedef struct kgi_mmio_region_s kgi_mmio_region_t;

typedef void kgi_mmio_set_offset_fn(kgi_mmio_region_t *r, kgi_size_t offset);

struct kgi_mmio_region_s {
	__KGI_RESOURCE

	kgi_u_t	access;			/* access widths allowed	*/
	kgi_u_t	align;			/* alignment requirements	*/

	kgi_size_t	size;		/* total size of the region     */
	kgi_aperture_t	win;		/* window aperture		*/

	kgi_size_t	offset;		/* window device-local address  */
	kgi_mmio_set_offset_fn	(*SetOffset);
};

/* Accelerators */
typedef enum {
	KGI_AS_IDLE,			/* has nothing to do		*/
	KGI_AS_FILL,			/* gets filled by application	*/
	KGI_AS_QUEUED,			/* is queued			*/
	KGI_AS_EXEC,			/* being executed		*/
	KGI_AS_WAIT			/* wait for execution to finish	*/
} kgi_accel_state_t;

typedef enum {
	KGI_BF_USEMUTEX		= 0x00000001,	/* Manage buffer mutex	*/
} kgi_buffer_flags_t;

typedef struct {
	kgi_aperture_t	aperture;	/* context buffer aperture info	*/
} kgi_accel_context_t;

typedef struct kgi_accel_buffer_s kgi_accel_buffer_t;

#define __KGI_ACCEL_BUFFER  \
	kgi_accel_buffer_t *next;	/* next of same mapping		*/\
	kgi_aperture_t	aperture;	/* buffer aperture location	*/\
	void		*context;	/* mapping context		*/\
	kgi_u_t		priority;	/* execution priority		*/\
	wait_queue_head_t *executed;	/* wakeup when buffer executed	*/\
	struct {							  \
		kgi_accel_state_t	state;	/* current buffer state	*/\
		kgi_accel_buffer_t	*next;	/* next in exec queue	*/\
		kgi_size_t		size;	/* bytes to execute	*/\
	} execution;							  \
	kgi_mutex_t			mtx;	/* buffer mutex		*/\
	kgi_buffer_flags_t		flags;	/* buffer flags		*/

struct kgi_accel_buffer_s {
	__KGI_ACCEL_BUFFER
};

struct kgi_accel_s;

typedef void kgi_accel_init_fn(struct kgi_accel_s *a, void *ctx);
typedef	void kgi_accel_done_fn(struct kgi_accel_s *a, void *ctx);
typedef void kgi_accel_exec_fn(struct kgi_accel_s *a,
		kgi_accel_buffer_t *b);
typedef void kgi_accel_wait_fn(struct kgi_accel_s *a);

typedef enum {
	KGI_AF_DMA_BUFFERS	= 0x00000001,	/* uses DMA to exec buffers */
	KGI_AF_USEMUTEX		= 0x00000002	/* uses mutex               */
} kgi_accel_flags_t;

typedef struct kgi_accel_s {
	__KGI_RESOURCE

	kgi_accel_flags_t	flags;	/* accelerator flags		*/
	kgi_u_t		buffers;	/* recommended number buffers	*/
	kgi_u_t		buffer_size;	/* recommended buffer size	*/
	void		*context;	/* current context		*/
	kgi_u_t		context_size;	/* context buffer size		*/
	struct {
		kgi_accel_buffer_t *queue;	/* buffers to execute	*/
		void *context;			/* current context	*/
	} execution;			/* dynamic state		*/

	wait_queue_head_t	*idle;	/* wakeup when becoming idle	*/
	kgi_accel_init_fn	*Init;
	kgi_accel_done_fn	*Done;
	kgi_accel_exec_fn	*Exec;
	kgi_mutex_t		mtx;	/* accel resource mutex		*/
} kgi_accel_t;

/*
 * Shared Memory
 */
typedef struct kgi_shmem_context_s kgi_shmem_context_t;
typedef struct kgi_shmem_s kgi_shmem_t;

typedef kgi_error_t kgi_shmem_context_init_fn(kgi_shmem_t *shmem, 
		kgi_shmem_context_t *ctx, kgi_size_t size);

typedef void kgi_shmem_context_fn(kgi_shmem_t *shmem, kgi_shmem_context_t *ctx);
typedef void kgi_shmem_art_fn(kgi_shmem_t *shmem, kgi_aperture_t *range);
typedef void kgi_shmem_art_map_fn(kgi_shmem_t *shmem, kgi_aperture_t *dst,
		kgi_aperture_t *src);

struct kgi_shmem_context_s {
	kgi_aperture_t	aperture;	/* aperture window exported	*/
	kgi_aperture_t	art_memory;	/* address relocation table (art) */
	kgi_aperture_t	null_page;	/* 'null' (invalid) memory page	*/
};

struct kgi_shmem_s {
	__KGI_RESOURCE

	kgi_size_t	aperture_size;	/* (maximum) aperture size	*/
	void		*ctx;		/* current context		*/
	kgi_size_t	ctx_size;	/* context struct size		*/
	kgi_shmem_context_init_fn	*ContextInit;
	kgi_shmem_context_fn		*ContextDone;
	kgi_shmem_context_fn		*ContextMap;
	kgi_shmem_context_fn		*ARTInit;
	kgi_shmem_art_fn		*ARTFlush;
	kgi_shmem_art_map_fn		*ARTMapPages;
	kgi_shmem_art_fn		*ARTUnmapPages;
};

/*
 * command resources
 */
typedef struct kgi_marker_s kgi_marker_t;
struct kgi_marker_s {
	__KGI_RESOURCE

	kgi_marker_mode_t	modes;	/* possible operation modes	*/
	kgi_u8_t		shapes;	/* number of shapes		*/
	kgi_u8_coord_t		size;	/* pattern size			*/
	void (*SetMode)(kgi_marker_t *marker, kgi_marker_mode_t mode);
	void (*Select)(kgi_marker_t *marker, kgi_u_t shape);
	void (*SetShape)(kgi_marker_t *marker, kgi_u_t shape,
		kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
		const kgi_rgb_color_t *color);

	void (*Show)(kgi_marker_t *marker, kgi_u_t x, kgi_u_t y);
	void (*Hide)(kgi_marker_t *marker);
	void (*Undo)(kgi_marker_t *marker);
	void (*Read)(kgi_marker_t *marker, kgi_u_t *x, kgi_u_t *y);
};

typedef struct kgi_text16_s kgi_text16_t;
struct kgi_text16_s {
	__KGI_RESOURCE

	kgi_ucoord_t	size;		/* visible text cells	*/
	kgi_ucoord_t	virt;		/* virtual text cells	*/
	kgi_ucoord_t	cell;		/* dots per text cell	*/
	kgi_ucoord_t	font;		/* dots per font cell	*/
	kgi_endian_t	bitendian;	/* font bit endianess   */

	void (*PutText16)(kgi_text16_t *text16, kgi_u_t offset,
			  const kgi_u16_t *text, kgi_u_t count);

	void (*LoadFont)(kgi_text16_t *text16, kgi_u_t page, kgi_u_t size,
			 kgi_u8_t *data, kgi_u_t ch, kgi_s_t count);
	void (*SaveFont)(kgi_text16_t *text16, kgi_u_t page, kgi_u_t size,
			 kgi_u8_t *data, kgi_u_t ch, kgi_s_t count);
	void (*ShowFont)(kgi_text16_t *text16, kgi_u_t page);
};

typedef struct kgi_clut_s kgi_ilut_t;	/* image look up table		*/
typedef struct kgi_clut_s kgi_alut_t;	/* attribute look up table	*/

typedef struct kgi_clut_s kgi_clut_t;	/* color look up table		*/
struct kgi_clut_s {
	__KGI_RESOURCE

	kgi_u_t tables;
	kgi_u_t entries;

	void (*Select)(kgi_clut_t *lut, kgi_u_t table);
	void (*Set)(kgi_clut_t *lut, kgi_u_t table, kgi_u_t idx,
		kgi_u_t count, kgi_attribute_mask_t am, const kgi_u16_t *data);
	void (*Get)(kgi_clut_t *lut, kgi_u_t table, kgi_u_t idx,
		kgi_u_t count, kgi_attribute_mask_t am, const kgi_u16_t *data);
};

typedef struct kgi_tlut_s kgi_tlut_t;	/* texture look up table	*/
struct kgi_tlut_s {
	__KGI_RESOURCE

	void (*Select)(kgi_tlut_t *tlut, kgi_u_t table);
	void (*Set)(kgi_tlut_t *tlut, kgi_u_t table, kgi_u_t idx,
		kgi_u_t count, const void *data);
};

#else /* _KERNEL */

/*
 * Prevent userspace from direct access to
 * internal data structures
 */
#define kgi_resource_t void

#endif /* ! _KERNEL */

#endif /* _kgi_kgires_h */
