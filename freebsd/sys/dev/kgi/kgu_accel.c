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
#define	KGI_DBG_LEVEL	1
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_VM
#define KGI_SYS_NEED_MUTEX
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>
#include <dev/kgi/accel.h>

/*
 * Accelerator resource mappings
 */

static void 
graph_accel_removepages(vm_area_t vma, graph_accel_buffer_t *buf, int free)
{
	vm_page_t m;
	
	/*
	 * Remove the current buffer from the process memory if free is TRUE.
	 * Any thread then hitting these pages will pagefault.
	 * XXX Suboptimal!
	 */
	while ((m = TAILQ_FIRST(&buf->memq)) != 0) {
		/* The page associated to the current buffer is removed */
		TAILQ_REMOVE(&buf->memq, m, kgiq);

		if (free) {
			/* Remove the page from vm area it is referenced by */
			kgi_pager_remove(vma, m);
		}
	}
}

/*
 * In the Linux version, there are only pagefaults on buffer
 * boundaries.
 *
 * Handle page faults for ACCEL mappings
 *
 * m is the page allocated by the pager to hold paddr. This enables
 * link between buffers and VM pages.
 *
 * ooffset is a number of bytes rounded to the page size of the
 * platform. It is an absolute offset in the accel.
 *
 * paddr the result is the physical address of the "device" obtained
 * from the ooffset given.
 */
static int 
graph_accel_nopage(vm_area_t vma, vm_page_t m, vm_offset_t ooffset,
			      vm_paddr_t *paddr, int prot)
{
	graph_accel_mapping_t *map;
	kgi_accel_t *accel;
	unsigned long nstart, nend;
	graph_accel_buffer_t *buf = NULL;

	map = (graph_accel_mapping_t *) VM(private_data);
	if (map == NULL) {
		KRN_DEBUG(1, "invalid (cloned ?) vma %p, sending signal", vma);
		panic("don't know how to signal...");	/* XXX */
	}
	KRN_ASSERT(vma == map->vma);

	accel = (kgi_accel_t *) map->resource;
	KRN_ASSERT(KGI_RT_ACCELERATOR == accel->type);

	KRN_DEBUG(3, "accel_nopage @%.8x, vma %p, %x", (kgi_u32_t)ooffset,
		  vma, prot);

	/*
	 * The page ooffset must be in the current buffer (no exec)
	 * or in the next buffer, but nothing else.
	 */
	if (ooffset < map->buf_offset)
		return (-1);
	
	if (ooffset < (map->buf_offset + map->buf_size))
		goto pagefound;

	/*
	 * Compute the start/end offsets of the buffer next to the
	 * current one.
	 */
	nstart = (map->buf_offset + map->buf_size) & map->buf_mask;
	nend = nstart + map->buf_size;
	if (! ((nstart <= ooffset) && (ooffset < nend))) {
		KRN_DEBUG(1, "ooffset %.8lx not in %.8lx-%.8lx for next buffer",
			(u_long)ooffset, nstart, nend);
		return (-1);
	}

	buf = map->buf_current;

	/*
	 * Lock the mutex of the next buffer to be sure to own
	 * it immediatly when running again
	 */
	kgi_mutex_lock(&buf->next->mtx);

	/*
	 * Wait for the next buffer to be idle before starting
	 * the execution of the current one. See later.
	 */
	while (buf->next->execution.state != KGI_AS_IDLE) {
		buf->next->flags |= KGI_BF_USEMUTEX;
		kgi_mutex_wait(&buf->next->mtx);
	}

	KRN_ASSERT(KGI_AS_FILL == buf->execution.state);
	KRN_DEBUG(3, "exec buffer %.8x", buf->aperture.phys);

	kgi_mutex_lock(&buf->mtx);

	/* Remove all pages of this buffer = VM_PROT_NONE */
	graph_accel_removepages(vma, buf, 1 /* yes, free */);
	
	/* Start "execution" of the buffer */
	buf->execution.size = 
		(ooffset == nstart) ? map->buf_size : (ooffset - nstart);

	accel->Exec(accel, (kgi_accel_buffer_t *)buf);

	kgi_mutex_unlock(&buf->mtx);

	/* Go to the next buffer */
	map->buf_current = (graph_accel_buffer_t *)buf->next;

	/*
	 * We are sure it is idle since we have waited for it.
	 * See above.
	 */
	KRN_ASSERT(KGI_AS_IDLE == buf->execution.state);
	buf->execution.state = KGI_AS_FILL;

	map->buf_offset = nstart;
	
	kgi_mutex_unlock(&buf->next->mtx);

 pagefound:
	/* Attach the page to the buffer */
	TAILQ_INSERT_TAIL(&buf->memq, m, kgiq);
	*paddr = buf->aperture.phys + (ooffset - map->buf_offset);

	return (0);
}

static void 
graph_accel_close(vm_area_t vma)
{
	graph_accel_mapping_t *map = (graph_accel_mapping_t *) VM(private_data);
	kgi_accel_t *accel = (kgi_accel_t *) map->resource;
	graph_accel_buffer_t *buf = NULL;

	map = (graph_accel_mapping_t *) VM(private_data);
	if (map == NULL) 
		return;
	KRN_ASSERT(map->vma == vma);

	accel = (kgi_accel_t *) map->resource;
	KRN_ASSERT(KGI_RT_ACCELERATOR == accel->type);

	KRN_DEBUG(1, "accel_unmap vma %p, map %p", vma, map);

	/* Search executable buffers among all its buffers.
	 */
	buf = (graph_accel_buffer_t *)map->buf_current->next;
	while ((buf->execution.next == NULL) &&
	       (buf != map->buf_current)) {
		buf = (graph_accel_buffer_t *)buf->next;
	}

	kgi_mutex_lock(&buf->mtx);

	/* Wait to have no more buffer to execute for this map */
	while (buf->execution.next != NULL) {
		buf->flags |= KGI_BF_USEMUTEX;
		kgi_mutex_wait(&buf->mtx);
	}

	buf->execution.state = KGI_AS_IDLE;

	accel->Done(accel, buf->context);

	/*
	 * free context, buffers and buffer infos
	 */
	buf = map->buf_current;
	kgi_kfree(buf->context);

	KRN_ASSERT(buf);
	KRN_ASSERT(buf->next);
	do {
		kgi_accel_buffer_t *next = buf->next;
		
		/*
		 * Remove the pages from the buf list, but keep
		 * them unfreed yet (already done?)
		 */
		graph_accel_removepages(vma, buf, 0 /* don't free */);

		KRN_DEBUG(1, "freeing %.8x with order %i",
			  buf->aperture.virt, map->buf_order);

		/* Free the contiguous memory of the HW mapping */
		kgi_cfree((void *)buf->aperture.virt, SIZ(map->buf_order));

		/* Free the buffer */
		kgi_kfree(buf);

		buf = (graph_accel_buffer_t *)next;

	} while (buf != map->buf_current);

	/*
	 * Don't unmap VM pages of the map here,
	 * the pager will do (has done?!) the job
	 */

	/* Reset the buffer of the map */
	map->buf_current = NULL;

	/* Unblock threads waiting for the termination of this buffer */
	kgi_mutex_signal(&buf->mtx, 1 /* all threads */);

	kgi_mutex_unlock(&buf->mtx);

	/* Reset the map of the vma */
	VM(private_data) = NULL;
}

/* XXX lock? */
static void 
graph_accel_unmap(vm_area_t vma)
{
	graph_accel_mapping_t *map = (graph_accel_mapping_t *) VM(private_data);
	graph_accel_buffer_t *buf;

	/* Reset the buffer queues of pages */
	buf = map->buf_current;
	do {
		kgi_accel_buffer_t *next = buf->next;

		/* Reset the queue of pages */
		TAILQ_INIT(&buf->memq);

		buf = (graph_accel_buffer_t *)next;

	} while (buf != map->buf_current);

	/* Free every pages of the map */
	kgi_pager_remove_all(vma);
}

static struct vm_operations_struct graph_accel_vmops =
{
	.open	=	NULL,
	.close	=	graph_accel_close,
	.nopage	=	graph_accel_nopage,
	.unmap  =	graph_accel_unmap,
};

/*
 * The area to be mapped is subdivided into buffers
 */
int 
graph_accel_mmap(vm_area_t vma, graph_mmap_setup_t *mmap_setup,
	  graph_mapping_t **the_map)
{
	kgi_accel_t *accel = (kgi_accel_t *)mmap_setup->resource;
	unsigned long min_order, max_order, priority, order, buffers, i;
	graph_accel_buffer_t *buf[16];
	kgi_accel_context_t *context = NULL;
	graph_accel_mapping_t *map = NULL;

	KRN_DEBUG(1, "mapping buffer %.8x to vma %p",
		  map->buf_current->aperture.phys, vma);

	min_order = mmap_setup->request.private.accel.min_order;
	order = max_order = mmap_setup->request.private.accel.max_order;
	buffers = mmap_setup->request.private.accel.buffers;
	priority = mmap_setup->request.private.accel.priority;

	memset(buf, 0, sizeof(buf));

	if (buffers > sizeof(buf)/sizeof(buf[0])) {
		KRN_ERROR("too many buffers (%li) requested", buffers);
		return (KGI_EINVAL);
	}

	if (min_order > max_order) {
		KRN_ERROR("invalid buffer size range (min %li, max %li)",
 			min_order, max_order);
		return (KGI_EINVAL);
	}

	/*
	 * XXX check for process limits (don't allow allocation of too
	 * XXX much memory).
	 *
	 *
	 * allocate mapping and init accelerator context
	 */
	if (!(context = kgi_kmalloc(accel->context_size))) {
		KRN_ERROR("failed to allocate accelerator context");
		goto no_memory; 
	}
	memset(context, 0, accel->context_size);

	context->aperture.size = accel->context_size;
	context->aperture.virt = (kgi_virt_addr_t)context;
	context->aperture.bus  = virt_to_phys(context);
	context->aperture.phys = virt_to_phys(context);

	accel->Init(accel, context);

	/* allocate buffer info */
	KRN_TRACE(1, memset(buf, 0, sizeof(buf)));
	for (i = 0; i < buffers; i++) {
		/* Reserve the among of memory including graph_accel stuff */
		buf[i] = kgi_kmalloc(sizeof(graph_accel_buffer_t));
		if (buf[i]) {
			memset(buf[i], 0, sizeof(*buf[i]));
			continue;
		}

		KRN_ERROR("failed to allocate buffer infos");
		goto no_memory;
	}

	/*
	 * allocate DMA buffers. These must be contiguous in physical
	 * address space and all of same size. We try beginning with
	 * max_order and check for smaller ones if this fails.
	 */
	do {
		for (i = 0; buf[i]->aperture.virt && (i < buffers); i++) {
			KRN_DEBUG(1, "freeing %i with order %li",
				buf[i]->aperture.virt, order + 1);
			kgi_cfree((void *)buf[i]->aperture.virt, SIZ(order + 1));
		}

		KRN_DEBUG(1, "trying %i byte buffers", SIZ(order));
		for (i = 0; i < buffers; i++) {
			buf[i]->aperture.virt = (kgi_virt_addr_t)kgi_cmalloc(SIZ(order));
			if (!buf[i]->aperture.virt) {
				break;
			}
			KRN_DEBUG(1, "allocated %i with order %li",
				buf[i]->aperture.virt, order);
		}

	} while ((i != buffers) && (--order >= min_order));
	if (i != buffers) {
		KRN_ERROR("failed to allocate DMA buffers");
		goto no_memory;
	}

	/* arrange buffers in circular list and attach to mapping. */
	for (i = 0; i < buffers; i++) {
		buf[i]->next = (kgi_accel_buffer_t *)buf[((i + 1) < buffers) ? (i + 1) : 0];
		buf[i]->priority = priority;
		buf[i]->context  = context;

		buf[i]->execution.next	= NULL;
		buf[i]->execution.state	= KGI_AS_IDLE;
		buf[i]->execution.size	= 0;

		buf[i]->aperture.size = SIZ(order);
		buf[i]->aperture.phys = virt_to_phys(buf[i]->aperture.virt);
		buf[i]->aperture.bus  = virt_to_phys(buf[i]->aperture.virt);

		TAILQ_INIT(&buf[i]->memq);
	}

	if (!(map = kgi_kmalloc(sizeof(*map)))) {
		KRN_ERROR("failed to allocate accel map");
		goto no_memory;
	}
	memset(map, 0, sizeof(*map));

	map->buf_current = buf[0];
	map->buf_order = order;
	map->buf_size = SIZ(order);
	map->buf_mask = (buffers - 1) << (order + PAGE_SHIFT);

	map->buf_current->execution.state = KGI_AS_FILL;

	vma->vm_ops = &graph_accel_vmops;

	/* Return the obtained map the upper layers */
	*the_map = (graph_mapping_t *)map;

	return (KGI_EOK);

no_memory:
	KRN_ERROR("accel: no memory");
	if (context) {
		accel->Done(accel, context);
		kgi_kfree(context);
	}

	order++;
	for (i = 0; buf[i] && (i < buffers); i++) {
		if (buf[i]->aperture.virt) {
			KRN_DEBUG(1, "freeing %.8x with order %li",
				buf[i]->aperture.virt, order);
			kgi_cfree((void *)buf[i]->aperture.virt, SIZ(order));
		}
		if (buf[i])
			kgi_kfree(buf[i]);
	}
	return (KGI_ENOMEM);
}

