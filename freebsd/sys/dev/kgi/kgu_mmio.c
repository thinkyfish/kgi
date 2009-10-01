/*-
 * Copyright (c) 1995-1997 Andreas Beck
 * Copyright (c) 1995-2000 Steffen Seeger
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	3
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_VM
#define KGI_SYS_NEED_MUTEX
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>
#include <dev/kgi/mmio.h>

/*
 * MMIO resource mapping
 */

/*
 * Handle page faults for MMIO mappings
 *
 * m is the page allocated by the pager to hold paddr (ignored by mmio
 * mappings since the vma/object already list the pages).
 *
 * ooffset is a number of bytes rounded to the page size of the
 * platform. It is an absolute offset in the mmio.
 *
 * paddr the result is the physical address of the "device" obtained
 * from the ooffset given.
 */
static int 
graph_mmio_nopage(vm_area_t vma, vm_page_t m, vm_offset_t ooffset,
	  vm_paddr_t *paddr, int prot)
{
	graph_mmio_mapping_t *map;
	kgi_mmio_region_t *mmio;
	kgi_u32_t offset;

 Retry:
	kgi_mutex_lock(&kgi_lock);

	map = (graph_mmio_mapping_t *) vma->vm_private_data;
	if (map == NULL) {
		KRN_ERROR("invalid (cloned ?) vma %p, sending signal", vma);
		panic("don't know how to signal...");	/* XXX */
	}

	mmio = (kgi_mmio_region_t *) map->resource;
	KRN_ASSERT((mmio->type & KGI_RT_MASK) == KGI_RT_MMIO);

	if (!ooffset)
		KRN_DEBUG(3, "mmio_nopage @%.8x, vma %p, %x", ooffset, vma, prot);

	if (map->device->kgi.flags & KGI_DF_FOCUSED) {
		switch(map->type) {
			/*
			 * Linear memory mapped linear. The physical address
			 * returned for the ooffset given as argument is the
			 * physical base address of the memory + the ooffset
			 * in bytes
			 */
		case GRAPH_MM_LINEAR_LINEAR:
			map->offset = offset = 0;
			*paddr = mmio->win.phys + ooffset;
			break;
			/*
			 * Paged memory mapped linear. The physical address
			 * returned is the physical base address + the offset
			 * withing the HW window
			 */
		case GRAPH_MM_PAGED_LINEAR:
			offset = ooffset / mmio->win.size;
			map->offset = ooffset % mmio->win.size;
			*paddr = mmio->win.phys + map->offset;
			break;
		case GRAPH_MM_PAGED_PAGED:
		case GRAPH_MM_LINEAR_PAGED:
		default:
			KRN_INTERNAL_ERROR;
			panic("don't know how to signal...");
		}

		if (!ooffset)
			KRN_DEBUG(3, "mmio_nopage @%.8x, remap vma %p, "
				  "to '%s' phys %x, offset %x",
				  ooffset, map->vma, mmio->name, *paddr, offset);

		/*
		 * Check if we have to move the HW window. If we have,
		 * the previously mapped window has to be unmapped
		 */
		if (offset != mmio->offset) {
			/* XXX Not tested */
			graph_unmap_resource((graph_mapping_t *) map);

			if (mmio->SetOffset) {
				mmio->SetOffset(mmio, offset);
			} else {
				mmio->offset = offset;
			}
		}

		kgi_mutex_unlock(&kgi_lock);
		return (0);

	} else {
		KRN_DEBUG(2, "Not focused, going to sleep!");

		/*
		 * The thread is trying to access an area while the
		 * device is not focused --> block it
		 */
		msleep(map->device, &kgi_lock.mutex, PDROP | PVM, "mmiosleep", 0);
		goto Retry;
	}
}

/*
 * graph_mmio_unmap() has to unmap a mmio mapping. As we do not allow
 * partial mapping of MMIO regions, we consequently have to deny partial
 * unmapping. This is ensured in mm/mmap.c. We only have to remove our
 * local mapping state info because the page tables are fixed in mm/mmap.c.
 */
static void 
graph_mmio_close(vm_area_t vma)
{
	/* Remove / free pages of the VM area */
	kgi_pager_remove_all(vma);

	/* Reset the map of the vma */
	vma->vm_private_data = NULL;
}

/*
 * Unmap the MMIO map. No more access shall be permitted to
 * this map.
 */
static void 
graph_mmio_unmap(vm_area_t vma)
{
	/* Just remove all the pages of the map */
	kgi_pager_remove_all(vma);
}

static struct vm_operations_struct graph_mmio_vmops =
{
	.open	 =	NULL,
	.close	 =	graph_mmio_close,
	.nopage	 =	graph_mmio_nopage,
	.unmap   =	graph_mmio_unmap,
};

int 
graph_mmio_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
	  graph_mapping_t **the_map)
{
	kgi_mmio_region_t *mmio = (kgi_mmio_region_t *)mmap_setup->resource;
	graph_mmio_mapping_t *map = NULL;
	vm_ooffset_t size = vma->vm_size;

	KRN_DEBUG(1, "mapping mmio '%s' to vma %p", mmio->name, vma);

	KRN_ASSERT(0 == (mmio->size % mmio->win.size));
	KRN_ASSERT(mmio->size >= mmio->win.size);

	if ((mmio->win.size & PAGE_MASK) ||
	    (size > mmio->size) ||
	    ((mmio->win.size == mmio->size) && (mmio->size % size)) ||
	    ((mmio->win.size != mmio->size) && (size < mmio->size) &&
	     (mmio->win.size != size))) {

		KRN_ERROR("mmap size %i, but region size %i, win.size %i",
			  (int)size, (int)mmio->size, (int)mmio->win.size);
		return (KGI_EINVAL);
	}

	KRN_DEBUG(1, "mmap size %i, region size %i, win.size %i OK",
		  (int)size, (int)mmio->size, (int)mmio->win.size);

	/* XXX warning determine protection flags here! */

	if (!(map = kgi_kmalloc(sizeof(*map)))) {
		KRN_ERROR("failed to allocate mmio map");
		return (KGI_ENOMEM);
	}
	memset(map, 0, sizeof(*map));

	map->prot = VM_PROT_READ | VM_PROT_WRITE; /* XXX | PROT_SHARED */;
	map->type = (mmio->win.size == mmio->size) /* linear region? */
		? ((size == mmio->size)
			? GRAPH_MM_LINEAR_LINEAR : GRAPH_MM_PAGED_LINEAR)
		: ((size == mmio->size)
			? GRAPH_MM_LINEAR_PAGED  : GRAPH_MM_PAGED_PAGED);

	vma->vm_ops = &graph_mmio_vmops;

	/* Return the obtained map the upper layers */
	*the_map = (graph_mapping_t *)map;

	return (KGI_EOK);
}
