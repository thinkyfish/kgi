/*-
 * Copyright (C) 1998-2000 Steffen Seeger
 * Copyright (C) 2004 Nicholas Souchu 
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
 * /dev/graphic accel OS dependent definitions
 */

#ifndef _kgi_accel_h
#define	_kgi_accel_h

typedef struct
{
	__KGI_ACCEL_BUFFER

	/* Below, fields hidden to the graphic drivers */

	TAILQ_HEAD(, vm_page) memq;	/* list of resident pages */

} graph_accel_buffer_t;

/*
 * Accelerator mappings
 */
typedef struct
{
	__GRAPH_RESOURCE_MAPPING

	unsigned long	buf_offset;	/* current mapped buffer offset	*/
	unsigned long	buf_size;	/* size of the buffers		*/
	unsigned long	buf_mask;	/* mask to wrap buffer		*/
	unsigned int	buf_order;	/* ln2(buffer_size)-PAGE_SHIFT	*/

	graph_accel_buffer_t *buf_current;	/* list of buffers		*/

} graph_accel_mapping_t;

#define SIZ(order) (1 << (order + PAGE_SHIFT))
#define VM(field) (vma->vm_##field)

extern int graph_accel_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
			    graph_mapping_t **the_map);

#endif /* #ifndef _kgi_accel_h */
