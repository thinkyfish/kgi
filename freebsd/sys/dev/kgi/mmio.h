/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2004 Nicholas Souchu
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
 * /dev/graphic mmio OS dependent definitions
 */

#ifndef _KGI_MMIO_H_
#define	_KGI_MMIO_H_

/*
 * MMIO mappings
 *
 * /dev/graphic provides a virtualized view to kgi_mmio_regions available
 * in a particular kgi_mode. Each kgi_mmio_region may be mapped several
 * times, possibly by several processes.
 * Per process, we keep a linked list of the various mappings (->next).
 * Additionally, all mappings of a particular kgi_mmio_region are kept
 * in a circular linked list (->other).
 */

typedef enum {
	GRAPH_MM_INVALID = 0,
	GRAPH_MM_LINEAR_LINEAR,
	GRAPH_MM_LINEAR_PAGED,
	GRAPH_MM_PAGED_LINEAR,
	GRAPH_MM_PAGED_PAGED,
	GRAPH_MM_LAST
} graph_mmio_maptype_t;

typedef struct {
	__GRAPH_RESOURCE_MAPPING
	graph_mmio_maptype_t	type;	/* mapping type			*/
	unsigned long 		offset;	/* offset for paged mappings	*/
	unsigned int		prot;	/* page protection flags	*/
} graph_mmio_mapping_t;

extern int graph_mmio_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
			   graph_mapping_t **map);

#endif /* !_KGI_MMIO_H_ */
