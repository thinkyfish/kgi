/* **********************************************************************
 *
 *   A P I  -  Matrox DMA buffers manipulation
 *
 * **********************************************************************/

#ifndef _MGA_ACCEL_DMA_H
#define _MGA_ACCEL_DMA_H

/*
**    Needed header files
*/
#include <kgi/kgi.h>

/* *********************************************
 *    Matrox vertex data structures
 * *********************************************/

typedef float mga_vertex_float_t;

typedef struct mga_vertex_color_s {
  kgi_u8_t blue, green, red, alpha;
} mga_vertex_color_t;

typedef struct mga_vertex_s {
  mga_vertex_float_t x; /* screen coordinates */
  mga_vertex_float_t y;
  mga_vertex_float_t z; /* Z buffer depth, in 0.0 - 1.0 */
  mga_vertex_float_t rhw; /* 1/w (reciprocal of homogeneous w) */
  mga_vertex_color_t color; /* vertex color */
  mga_vertex_color_t specular; /* specular component. Alpha is fog factor */
  mga_vertex_float_t tu0; /* texture coordinate stage 0 */
  mga_vertex_float_t tv0;
} mga_vertex_t;

typedef struct mga_vertex2_s {
  mga_vertex_float_t x; /* screen coordinates */
  mga_vertex_float_t y;
  mga_vertex_float_t z; /* 0.0 - 1.0 */
  mga_vertex_float_t rhw; /* 1/w (reciprocal of homogeneous w) */
  mga_vertex_color_t color; /* vertex color */
  mga_vertex_color_t specular; /* specular component. Alpha is fog factor */
  mga_vertex_float_t tu0; /* texture coordinate stage 0 */
  mga_vertex_float_t tv0;
  mga_vertex_float_t tu1; /* texture coordinate stage 1 */
  mga_vertex_float_t tv1;
} mga_vertex2_t;


/* *********************************************
 *    Matrox raw DMA buffer data structures
 * *********************************************/

/*
** Data structures
*/

/* Type of buffer */
typedef enum
{
  MGA_DMA_GENERAL_PURPOSE,
  MGA_DMA_VERTEX_TRIANGLE_LIST,
  MGA_DMA_VERTEX_TRIANGLE_STRIP,
  MGA_DMA_VERTEX_TRIANGLE_FAN
} mga_dma_buffer_type_t;

/* Returns a string for printing */
#define __MGA_DMA_BUFFER_TYPE_STRING(e,v,other) \
 (((e) == (v)) ? #v : (other))
#define MGA_DMA_BUFFER_TYPE_STRING(t)                             \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_GENERAL_PURPOSE,       \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_LIST,  \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_STRIP, \
  __MGA_DMA_BUFFER_TYPE_STRING((t),MGA_DMA_VERTEX_TRIANGLE_FAN,   \
  "unknown"))))

/* Buffer data structure */
typedef struct _mga_dma_buffer_s mga_dma_buffer_t;

#define MGA_DMA_BUFFER_MIN_SIZE 1

/* *************************************************
 *    Matrox raw DMA buffer manipulation functions
 * *************************************************/

/*
** Note that these functions do *not* do error checking (esp. wrt
** overflows). You need to do these checks manually in the calling
** program. This is for performance reasons...
*/

/* First time initialization of a buffer */
static void mga_dma_buffer_init(mga_dma_buffer_t *buf,
				kgi_u32_t *start,
				kgi_u32_t *past_of_end);
/* Reset an existing buffer */
static inline void mga_dma_buffer_reset(mga_dma_buffer_t *buf,
					mga_dma_buffer_type_t type);
/* Returns the remaining space in a buffer (unit is buffer type dependent) */
static inline kgi_u_t mga_dma_buffer_space(mga_dma_buffer_t *buf);
/* Returns the size of the used space in a buffer (in 32bits dwords) */
static inline kgi_u_t mga_dma_buffer_size(mga_dma_buffer_t *buf);
/* Output of register (general purpose DMA) */
static inline void mga_dma_buffer_out_reg(mga_dma_buffer_t *buf,
					  kgi_u32_t val, kgi_u32_t reg);
#define DMA_BLOCK_FUNCTIONS 1
#if DMA_BLOCK_FUNCTIONS
static inline void mga_dma_buffer_out_block(mga_dma_buffer_t *buf,
					    kgi_u32_t val1, kgi_u32_t reg1,
					    kgi_u32_t val2, kgi_u32_t reg2,
					    kgi_u32_t val3, kgi_u32_t reg3,
					    kgi_u32_t val4, kgi_u32_t reg4);
static inline void mga_dma_buffer_pad_to_block(mga_dma_buffer_t *buf);
#endif /* DMA_BLOCK_FUNCTIONS */
/* Output of vertices (VERTEX DMA buffer) */
static inline void mga_dma_buffer_out_vertex(mga_dma_buffer_t *buf,
					     mga_vertex_t *v);
static inline void mga_dma_buffer_out_triangle(mga_dma_buffer_t *buf,
					       mga_vertex_t *v1,
					       mga_vertex_t *v2,
					       mga_vertex_t *v3);
/* Printing functions */
static inline void mga_dma_buffer_fprintf(FILE *stream, mga_dma_buffer_t* buf);
static inline void mga_dma_buffer_fprintf_content(FILE *stream,
						  mga_dma_buffer_t* buf);
static inline void mga_dma_buffer_printf(mga_dma_buffer_t* buf);
static inline void mga_dma_buffer_printf_content(mga_dma_buffer_t* buf);



/* **************************************************************************
 *
 *  I M P L E M E N T A T I O N  -  Matrox DMA buffers manipulation
 *
 * **************************************************************************/

/* *********************************************
 *    Matrox raw DMA buffer
 * *********************************************/

/*
** Data structures
*/

/* Rank within a DMA block (1sr, 2nd, 3rd or 4th entry of the block)
 */
typedef enum
{
  MGA_DMA_FIRST_ENTRY = 0,
  MGA_DMA_SECOND_ENTRY = 1,
  MGA_DMA_THIRD_ENTRY = 2,
  MGA_DMA_FORTH_ENTRY = 3
} _mga_dma_block_entry_t;
#define MGA_DMA_BLOCK_ENTRY_MASK 0x3

/* Size of a block in dwords */
#define _MGA_DMA_BLOCK_SIZE 5
/* Number of useful entries */
#define _MGA_DMA_BLOCK_ENTRIES_NUMBER 4

/* Returns a string for printing
 */
#define __MGA_DMA_BLOCK_ENTRY_STRING(e,v,other) \
 (((e) == (v)) ? #v : (other))
#define _MGA_DMA_BLOCK_ENTRY_STRING(e) \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_FIRST_ENTRY, \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_SECOND_ENTRY,\
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_THIRD_ENTRY, \
  __MGA_DMA_BLOCK_ENTRY_STRING((e),MGA_DMA_FORTH_ENTRY, \
  "unknown"))))

/*
 * Each DMA buffer contains an initial 32bits tag indicating its type.
 * A general DMA buffer is made of consecutive 5-dword blocks. In each
 * block, the initial dword contains 4 8bits register indexes, and the
 * next 4 dwords contain the values to store in these registers.
 * A vertex buffer is made of consecutive mga_vertex_t or mga_vertex2_t
 * data structures.
 */

struct _mga_dma_buffer_s
{
 /* Start address of buffer area */
  kgi_u32_t *mem_start;
  /* (Past-of-)End address */
  kgi_u32_t *mem_end;
  /* Current type */
  mga_dma_buffer_type_t type;

  union
  {
    struct
    {
      /* Current pointer: beginning of block */
      kgi_u32_t *block;
      _mga_dma_block_entry_t entry;
    } regs;

    mga_vertex_t* vertex;
    
  } current;

  /* Next buffer in the ring */
  mga_dma_buffer_t *next;
};

/*
** DMA buffer manipulation functions
**
** Note that these functions do *not* do error checking (esp. wrt
** overflows). You need to do these checks manually in the calling
** program. This is for performance reasons...
*/

/* reset a buffer for a specific type of usage
 */
static void mga_dma_buffer_reset(mga_dma_buffer_t *buf,
				 mga_dma_buffer_type_t type)
{
  switch (type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      /* tag the buffer */
      *(buf->mem_start) = MGAG_ACCEL_TAG_DRAWING_ENGINE;
      buf->current.regs.block = buf->mem_start + 1;
      buf->current.regs.entry = MGA_DMA_FIRST_ENTRY;
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      /* tag the buffer */
      *(buf->mem_start) = MGAG_ACCEL_TAG_WARP_TRIANGLE_LIST;
      buf->current.vertex = (mga_vertex_t*)(buf->mem_start + 1);
      break;
    default:
      fprintf(stderr, "Unknown reset buffer type (%i)", type);
      exit(1);
      break;
    }
  buf->type = type;
}

/* General purpose DMA */
static void mga_dma_buffer_init(mga_dma_buffer_t *buf,
				kgi_u32_t *start, kgi_u32_t *past_of_end)
{
  buf->mem_start = start;
  buf->mem_end = past_of_end;
  buf->next = NULL;
}

static kgi_u_t mga_dma_buffer_space(mga_dma_buffer_t *buf)
{
  kgi_u_t ret;
  switch (buf->type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      {
	kgi_u_t dword_size = (buf->mem_end - buf->current.regs.block);
	ret = (((dword_size / _MGA_DMA_BLOCK_SIZE)
		* _MGA_DMA_BLOCK_ENTRIES_NUMBER)
	       - buf->current.regs.entry)
	  + ((dword_size % _MGA_DMA_BLOCK_SIZE)
	     - (_MGA_DMA_BLOCK_SIZE - _MGA_DMA_BLOCK_ENTRIES_NUMBER))
	  - 1; /* TODO: Should remove (but careful with full buffers) */
      }
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      {
	kgi_u_t dword_size = (buf->mem_end - (kgi_u32_t*)buf->current.vertex);
	ret = dword_size / (3 * (sizeof(mga_vertex_t) >> 2));
      }
      break;
    default:
      fprintf(stderr, "Unknown buffer type (%i) in space()", buf->type);
      ret = -1;
      break;
    }
  return ret;
}

static kgi_u_t mga_dma_buffer_size(mga_dma_buffer_t *buf)
{
  kgi_u_t dword_size;
  switch (buf->type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      dword_size = (buf->current.regs.block - buf->mem_start)
	+ buf->current.regs.entry
	+ ((buf->current.regs.entry != MGA_DMA_FIRST_ENTRY) ? 1 : 0);
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      dword_size = ((kgi_u32_t*)buf->current.vertex - buf->mem_start);
      break;
    default:
      fprintf(stderr, "Unknown buffer type (%i) in size()", buf->type);
      dword_size = MGA_DMA_BUFFER_MIN_SIZE;
      break;
    }
  return dword_size;
}

static void mga_dma_buffer_out_reg(mga_dma_buffer_t *buf,
				   kgi_u32_t val, kgi_u32_t reg)
{
  /* Update the index */
  switch (buf->current.regs.entry)
    {
    case MGA_DMA_FIRST_ENTRY:
      *(buf->current.regs.block) = MGA_DMA(reg);
      *(buf->current.regs.block + 1) = val;
      buf->current.regs.entry = MGA_DMA_SECOND_ENTRY;
      break;
    case MGA_DMA_SECOND_ENTRY:
      *(buf->current.regs.block) |= MGA_DMA(reg) << 8;
      *(buf->current.regs.block + 2) = val;
      buf->current.regs.entry = MGA_DMA_THIRD_ENTRY;
      break;
    case MGA_DMA_THIRD_ENTRY:
      *(buf->current.regs.block) |= MGA_DMA(reg) << 16;
      *(buf->current.regs.block + 3) = val;
      buf->current.regs.entry = MGA_DMA_FORTH_ENTRY;
      break;
    case MGA_DMA_FORTH_ENTRY:
      *(buf->current.regs.block) |= MGA_DMA(reg) << 24;
      *(buf->current.regs.block + 4) = val;
      /* Advances to next block */
      buf->current.regs.block += _MGA_DMA_BLOCK_SIZE;
      buf->current.regs.entry = MGA_DMA_FIRST_ENTRY;
      break;
    }
}

#if DMA_BLOCK_FUNCTIONS
/* I really doubt that this really provide a good optimization
** compared to the above function when a good optimizing compiler
** is available. Hence, I postpone the implementation idea until
** I have time for experiments -- ortalo
*/
static void mga_dma_buffer_out_block(mga_dma_buffer_t *buf,
				     kgi_u32_t val1, kgi_u32_t reg1,
				     kgi_u32_t val2, kgi_u32_t reg2,
				     kgi_u32_t val3, kgi_u32_t reg3,
				     kgi_u32_t val4, kgi_u32_t reg4)
{
  if (buf->current.regs.entry == MGA_DMA_FIRST_ENTRY)
    {
      /* Faster case */
      *(buf->current.regs.block) = MGA_DMA(reg1) | (MGA_DMA(reg2) << 8)
	| (MGA_DMA(reg3) << 16) | (MGA_DMA(reg4) << 24);
      *(buf->current.regs.block + 1) = val1;
      *(buf->current.regs.block + 2) = val2;
      *(buf->current.regs.block + 3) = val3;
      *(buf->current.regs.block + 4) = val4;
      buf->current.regs.block += _MGA_DMA_BLOCK_SIZE;
    }
  else
    {
      /* Fall back */
      mga_dma_buffer_out_reg(buf, val1, reg1);
      mga_dma_buffer_out_reg(buf, val2, reg2);
      mga_dma_buffer_out_reg(buf, val3, reg3);
      mga_dma_buffer_out_reg(buf, val4, reg4);
    }
}

/* Pad the current buffer to sync on DMA boundary (useful if the above
** is).
*/
static void mga_dma_buffer_pad_to_block(mga_dma_buffer_t *buf)
{
  while (buf->current.regs.entry != MGA_DMA_FIRST_ENTRY)
    {
      mga_dma_buffer_out_reg(buf, 0, DMAPAD);
    }
}
#endif

/* Vertex functions
 */
static void mga_dma_buffer_out_vertex(mga_dma_buffer_t *buf, mga_vertex_t *v)
{
  memcpy(buf->current.vertex,v,sizeof(mga_vertex_t));
  ++(buf->current.vertex);
}

static void mga_dma_buffer_out_triangle(mga_dma_buffer_t *buf,
					mga_vertex_t *v1,
					mga_vertex_t *v2,
					mga_vertex_t *v3)
{
  mga_dma_buffer_out_vertex(buf,v1);
  mga_dma_buffer_out_vertex(buf,v2);
  mga_dma_buffer_out_vertex(buf,v3);
}

/* Printing functions
 */
static void mga_dma_buffer_fprintf(FILE *stream, mga_dma_buffer_t* buf)
{
  fprintf(stream, "Matrox DMA buffer:\n"
	  " start address: 0x%.8x\n"
	  " end address  : 0x%.8x\n"
	  " type         : %s (%i)\n"
	  " remaining space: %i (type-dependent)\n"
	  " used size    : %i (in dword) or %i (in bytes)\n"
	  " next address : 0x%.8x\n",
	  buf->mem_start, buf->mem_end,
	  MGA_DMA_BUFFER_TYPE_STRING(buf->type), buf->type,
	  mga_dma_buffer_space(buf),
	  mga_dma_buffer_size(buf),
	  4 * mga_dma_buffer_size(buf),
	  buf->next);
  switch (buf->type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      fprintf(stream, " general-purpose DMA specific data:\n"
	      "  current block: 0x%.8x\n"
	      "  current entry: %s (%i)\n",
	      buf->current.regs.block,
	      _MGA_DMA_BLOCK_ENTRY_STRING(buf->current.regs.entry),
	      buf->current.regs.entry);
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      fprintf(stream, "  vertex DMA specific data:\n"
	      "  current vertex pointer: %.8x\n",
	      buf->current.vertex);
      break;
    default:
      fprintf(stream, " no type-specific data\n");
      break;
    }
}

static void mga_dma_buffer_fprintf_content(FILE *stream, mga_dma_buffer_t* buf)
{
  mga_dma_buffer_fprintf(stream, buf);
  fprintf(stream, " DMA buffer content:\n"
	  "  buffer tag: %.8x\n",
	  *(buf->mem_start));
  switch (buf->type)
    {
    case MGA_DMA_GENERAL_PURPOSE:
      {
	kgi_u32_t *ptr = buf->mem_start + 1;
	while (ptr <= buf->current.regs.block)
	  {
	    int i;
	    fprintf(stream,"  %.8x-", ptr);
	    for (i = 0; i < _MGA_DMA_BLOCK_SIZE; i++)
	      if (ptr < buf->mem_end) /* Avoids to touch past the end */
		fprintf(stream," %.8x", *ptr++);
	    fprintf(stream,"\n");
	  }
      }
      break;
    case MGA_DMA_VERTEX_TRIANGLE_LIST:
      {
	mga_vertex_t *ptr = (mga_vertex_t*)(buf->mem_start + 1);
	while (ptr < buf->current.vertex)
	  {
	    fprintf(stream,
		    "  %.8x - x=%f y=%f z=%f\n"
		    "         rhw=%f tu0=%f tv0=%f\n"
		    "         color=RGBA(%i,%i,%i,%i)\n"
		    "         specular=RGBA(%i,%i,%i,%i)\n",
		    ptr,
		    ptr->x, ptr->y, ptr->z,
		    ptr->rhw, ptr->tu0, ptr->tv0,
		    ptr->color.red, ptr->color.green, ptr->color.blue,
		    ptr->color.alpha,
		    ptr->specular.red, ptr->specular.green, ptr->specular.blue,
		    ptr->specular.alpha);
	    ptr++;
	  }
      }
      break;
    default:
      fprintf(stream, " no type-specific data\n");
      break;
    }
}

static void mga_dma_buffer_printf(mga_dma_buffer_t* buf)
{
  mga_dma_buffer_fprintf(stdout, buf);
}

static void mga_dma_buffer_printf_content(mga_dma_buffer_t* buf)
{
  mga_dma_buffer_fprintf_content(stdout, buf);
}

#undef __MGA_DMA_BLOCK_ENTRY_STRING(e,v,other)
#undef _MGA_DMA_BLOCK_ENTRY_STRING(e)

#endif /* _MGA_ACCEL_DMA_H */
