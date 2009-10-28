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

#define KGI_SYS_NEED_VM
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>

/*
 * Helper functions
 */

/*
 * XXX Mapping stuff should be rewritten with queue macros
 */
void 
graph_add_mapping(graph_file_t *file, graph_mapping_t *map)
{
	unsigned int i;

	KRN_ASSERT(file);
	KRN_ASSERT(file->device);
	KRN_ASSERT(file->device->kgi.mode);
	KRN_ASSERT(map->resource);

	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {
		if (file->device->kgi.mode->resource[i] == map->resource) 
			break;
	}
	KRN_ASSERT(i < __KGI_MAX_NR_RESOURCES);

	file->refcnt++;

	map->file = file;
	if (file->mappings) {
		map->next = file->mappings->next;
		file->mappings->next = map;
	} else {
		map->next = map;
		file->mappings = map;
	}

	map->device = file->device;
	if (map->device->mappings[i]) {
		map->other = map->device->mappings[i]->other;
		map->device->mappings[i]->other = map;

	} else {
		map->other = map;
		map->device->mappings[i] = map;
	}
	KRN_DEBUG(1, "added mapping %p", map);
}

void 
graph_delete_mapping(graph_mapping_t *map)
{
	graph_mapping_t *prev;
	unsigned int i;

	KRN_DEBUG(1, "deleting mapping %p", map);
	KRN_ASSERT(map);
	KRN_ASSERT(map->file);
	KRN_ASSERT(map->device);

	/* delete mapping from mappings-for-same-file list */
	prev = map->next;
	while (prev->next != map) 
		prev = prev->next;

	if (map->file->mappings == map)
		map->file->mappings = (prev != map) ? prev : NULL;

	prev->next = map->next;

	/* delete from mappings-for-same-region list */
	prev = map->other;
	while (prev->other != map) 
		prev = prev->other;

	i = 0;
	while ((i < __KGI_MAX_NR_RESOURCES) &&
		(map->device->kgi.mode->resource[i] != map->resource)) {
		i++;
	}
	KRN_ASSERT(i < __KGI_MAX_NR_RESOURCES);

	if (map->device->mappings[i] == map)
		map->device->mappings[i] = (prev != map) ? prev : NULL;

	prev->other = map->other;

	map->file->refcnt--;

	KRN_TRACE(1, memset(map, 0, sizeof(*map)));
}

/*
 * Prevent access to all the VM pages associated to the object of this map.
 * The map here is just used to obtain the object.
 */
void 
graph_unmap_map(graph_mapping_t *map)
{
	vm_area_t vma = map->vma;

	if (!vma->vm_ops || !vma->vm_ops->unmap) 
		panic("Don't know how to unmap!!");

	KRN_DEBUG(3, "Unmapping map %p", map);

	/* This operation is resource dependent */
	vma->vm_ops->unmap(vma);
}

/*
 * Unmap mappings of the same resource than the one associated
 * to the given map. This typically happen when moving a HW window
 * while linear mapping it in the VM of the processes. When the HW
 * window is moved, any process that was previously pointing to
 * the window should pagefault again to update its VM
 */
void 
graph_unmap_resource(graph_mapping_t *map)
{

	graph_mapping_t *first = map;

	do {
		KRN_ASSERT(map->resource == first->resource);

		/* 
		 * The VM pages are deleted but the map is not
		 * deleted
		 */
		graph_unmap_map(map);
		map = map->other;
	} while (map != first);

	KRN_DEBUG(3, "mappings for region '%s' unmapped", map->resource->name);
}
