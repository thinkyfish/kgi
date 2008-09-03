/* ----------------------------------------------------------------------------
**	/dev/graphic mmio OS dependent definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**	Copyright (C)	2004		Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
**
** ----------------------------------------------------------------------------
**	
**	$FreeBSD$
**	
*/
#ifndef _kgi_mmio_h
#define	_kgi_mmio_h

/*	MMIO mappings
**
**	/dev/graphic provides a virtualized view to kgi_mmio_regions available
**	in a particular kgi_mode. Each kgi_mmio_region may be mapped several
**	times, possibly by several processes.
**	Per process, we keep a linked list of the various mappings (->next).
**	Additionally, all mappings of a particular kgi_mmio_region are kept
**	in a circular linked list (->other).
*/

typedef enum
{
	GRAPH_MM_INVALID = 0,
	GRAPH_MM_LINEAR_LINEAR,
	GRAPH_MM_LINEAR_PAGED,
	GRAPH_MM_PAGED_LINEAR,
	GRAPH_MM_PAGED_PAGED,

	GRAPH_MM_LAST

} graph_mmio_maptype_t;

typedef struct
{
	__GRAPH_RESOURCE_MAPPING

	graph_mmio_maptype_t	type;	/* mapping type			*/
	unsigned long 		offset;	/* offset for paged mappings	*/
	unsigned int		prot;	/* page protection flags	*/

} graph_mmio_mapping_t;

extern int graph_mmio_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
			   graph_mapping_t **map);

#endif /* #ifndef _kgi_mmio_h */
