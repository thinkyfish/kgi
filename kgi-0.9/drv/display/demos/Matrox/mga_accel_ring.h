/* **********************************************************************
 *
 *  A P I  -  Ring of Matrox DMA buffers
 *
 * **********************************************************************/

#ifndef _MGA_ACCEL_RING_H
#define _MGA_ACCEL_RING_H

/*
**    Needed header files
*/
#include <kgi/kgi.h>
#include "mga_accel_dma.h"

/* *********************************************
 *    DMA buffers ring data structure
 * *********************************************/

typedef struct _mga_dma_ring_s mga_dma_ring_t;

/* *********************************************
 *    DMA buffers ring operations
 * *********************************************/

/* Initialization of a ring */
static void mga_dma_init(mga_dma_ring_t *ring, kgi_u8_t *the_start,
			 kgi_u_t size, kgi_u_t no);
/* Flush a ring: executes current buffer and goes to next one */
static inline void mga_dma_flush(mga_dma_ring_t *ring,
				 mga_dma_buffer_type_t next_type);
/* Check type and available space (in type unit) in current buffer */
static inline void mga_dma_check(mga_dma_ring_t *ring, kgi_u_t no,
				 mga_dma_buffer_type_t next_type);
/* Register output: for general purpose DMA */
static inline void mga_dma_out_reg(mga_dma_ring_t *ring,
				   kgi_u32_t val, kgi_u32_t reg);
#if DMA_BLOCK_FUNCTIONS
static inline void mga_dma_out_block(mga_dma_ring_t *ring,
				     kgi_u32_t val1, kgi_u32_t reg1,
				     kgi_u32_t val2, kgi_u32_t reg2,
				     kgi_u32_t val3, kgi_u32_t reg3,
				     kgi_u32_t val4, kgi_u32_t reg4);
#endif /* DMA_BLOCK_FUNCTIONS */
/* Vertices output (VERTEX DMA mode) */
static inline void mga_dma_out_triangle(mga_dma_ring_t *ring,
					mga_vertex_t *v1,
					mga_vertex_t *v2,
					mga_vertex_t *v3);



/* **************************************************************************
 *
 *  I M P L E M E N T A T I O N  -  Ring DMA buffers operations
 *
 * **************************************************************************/

/* *********************************************
 *    DMA buffers ring data structures
 * *********************************************/

struct _mga_dma_ring_s
{
  kgi_u_t no_buffers;
  kgi_u_t buffer_size; /* in bytes */
  kgi_u32_t *start;

  mga_dma_buffer_t *current;

};

/* *********************************************
 *    DMA buffers ring operations
 * *********************************************/

#define _mga_dma_ring_malloc(x) malloc(x)

static void mga_dma_init(mga_dma_ring_t *ring, kgi_u8_t *the_start,
			 kgi_u_t size, kgi_u_t no)
{
  kgi_u32_t *start = (kgi_u32_t*)the_start;
  mga_dma_buffer_t *first;
  mga_dma_buffer_t *prev;
  kgi_u_t i;

  first = NULL;
  ring->no_buffers = no;
  ring->buffer_size = size >> 2;
  ring->start = start;
  /* Builds a circular ring of buffers */
  for (i = 0; i < no; i++)
    {
      mga_dma_buffer_t *buf;
      buf = (mga_dma_buffer_t*)_mga_dma_ring_malloc(sizeof(mga_dma_buffer_t));
      mga_dma_buffer_init(buf, start, start + ring->buffer_size);
      start += ring->buffer_size;
      if (first == NULL)
	first = buf;
      else
	prev->next = buf;
      prev = buf;
    }
  prev->next = first;
  /* Sets the first buffer as current */
  ring->current = first;
  /* does a reset on the first buffer */
  mga_dma_buffer_reset(ring->current, MGA_DMA_GENERAL_PURPOSE);
}

static void mga_dma_flush(mga_dma_ring_t *ring,
			  mga_dma_buffer_type_t next_type)
{
  kgi_u_t size = mga_dma_buffer_size(ring->current);
  if (size > MGA_DMA_BUFFER_MIN_SIZE)
    { /* Something to execute */
#if BUFFER_IN_MEMORY
      /* DO NOT EXECUTE A PRINT on the buffer (risk to overflow...) */
      mga_dma_buffer_printf_content(ring->current);
      //exit(1);
#endif
      /* Goes to next buffer */
      ring->current = ring->current->next;
      /* Touches the right place in new buffer to trigger exchange */
#warning full buffer should be offset trigger 0
      {
	kgi_u8_t *ptr = (kgi_u8_t*)ring->current->mem_start;
	ptr += size * 4;
	*ptr = 0x00;
      }
    }
  /* Resets new buffer (for general purpose DMA) */
  mga_dma_buffer_reset(ring->current, next_type);
}

static void mga_dma_check(mga_dma_ring_t *ring, kgi_u_t no,
			  mga_dma_buffer_type_t next_type)
{
  if (ring->current->type != next_type)
    mga_dma_flush(ring, next_type);
  if (mga_dma_buffer_space(ring->current) < no)
    mga_dma_flush(ring, next_type);
}

static void mga_dma_out_reg(mga_dma_ring_t *ring, kgi_u32_t val, kgi_u32_t reg)
{
  mga_dma_buffer_out_reg(ring->current, val, reg);
}

#if DMA_BLOCK_FUNCTIONS
static void mga_dma_out_block(mga_dma_ring_t *ring,
			      kgi_u32_t val1, kgi_u32_t reg1,
			      kgi_u32_t val2, kgi_u32_t reg2,
			      kgi_u32_t val3, kgi_u32_t reg3,
			      kgi_u32_t val4, kgi_u32_t reg4)
{
  mga_dma_buffer_pad_to_block(ring->current);
  mga_dma_buffer_out_block(ring->current,
			   val1, reg1, val2, reg2, val3, reg3, val4, reg4);
}
#endif /* DMA_BLOCK_FUNCTIONS */

static void mga_dma_out_triangle(mga_dma_ring_t *ring,
				 mga_vertex_t *v1,
				 mga_vertex_t *v2,
				 mga_vertex_t *v3)
{
  mga_dma_buffer_out_triangle(ring->current, v1, v2, v3);
}

#endif /* _MGA_ACCEL_RING_H */
