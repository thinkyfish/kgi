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
 * /dev/graphic accel OS dependent definitions
 */

#ifndef _KGI_ACCEL_H_
#define	_KGI_ACCEL_H_

typedef struct
{
	__KGI_ACCEL_BUFFER

	/* Below, fields hidden to the graphic drivers */

	TAILQ_HEAD(, vm_page) memq;	/* list of resident pages */

} graph_accel_buffer_t;

/*
 * Accelerator mappings
 */
typedef struct {
	__GRAPH_RESOURCE_MAPPING

	unsigned long	buf_offset;	/* current mapped buffer offset	*/
	unsigned long	buf_size;	/* size of the buffers		*/
	unsigned long	buf_mask;	/* mask to wrap buffer		*/
	unsigned int	buf_order;	/* ln2(buffer_size)-PAGE_SHIFT	*/

	graph_accel_buffer_t *buf_current; /* list of buffers		*/
} graph_accel_mapping_t;

#define SIZ(order) (1 << (order + PAGE_SHIFT))
#define VM(field) (vma->vm_##field)

extern int graph_accel_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
		graph_mapping_t **the_map);

#endif /* _KGI_ACCEL_H_ */
