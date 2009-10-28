/*-
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
 * KGI FreeBSD memory system layer
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_VM
#define KGI_SYS_NEED_USER
#include <dev/kgi/system.h>

#include <dev/kgi/kgidbg.h>

/*
 * No mutexes should be held (except for Giant) across functions which
 * access memory in userspace, such as copyin(9), copyout(9), uiomove(9),
 * fuword(9), etc.  No locks are needed when calling these functions.
 */

/*
 * Copy a kernel area to user process area.
 */
__kgi_u32_t
kgi_copy_to_user(void *to, const void *from, __kgi_u32_t n)
{

	if (copyout(from, to, n))
		return (0);

	return (n);
}

/*
 * Copy a user process area to a kernel area.
 */
__kgi_u32_t
kgi_copy_from_user(void *to, const void *from, __kgi_u32_t n)
{

	if (copyin(from, to, n))
		return (0);

	return (n);
}

/*
 * Map a physical address (buffer) into kernel virtual memory.
 * The size is truncate to a perfect number of pages.
 */
__kgi_virt_addr_t
kgi_map_buffer(__kgi_phys_addr_t paddr, __kgi_size_t size)
{
	__kgi_virt_addr_t vaddr;
	__kgi_phys_addr_t off;

	off = paddr - trunc_page(paddr);
	vaddr = (__kgi_virt_addr_t)pmap_mapdev(paddr - off, size + off);

	return (vaddr + off);
}

/*
 * Unmap a physical address (buffer) from kernel virtual memory.
 */
void
kgi_unmap_buffer(__kgi_virt_addr_t vaddr, __kgi_size_t size)
{

	pmap_unmapdev((vm_offset_t)vaddr, (vm_size_t)size);
}

/*
 * Allocate kernel memory of type M_KGI.
 */
void *
kgi_kmalloc(__kgi_size_t size)
{
	void *ptr;

	ptr = malloc(size, M_KGI, M_NOWAIT);
	KRN_DEBUG(3, "Allocating %p (%d)", (void *)ptr, size);
	return (ptr);
}

/*
 * Release kernel memory of type M_KGI.
 */
void 
kgi_kfree(const void *ptr)
{

	KRN_DEBUG(3, "Freeing %p", (void *)ptr);
	free((void *)ptr, M_KGI);
}

/*
 * Allocate a contiguous memory.
 */
void *
kgi_cmalloc(__kgi_size_t size)
{

	return (contigmalloc(size, M_KGI, M_NOWAIT, 0, ~0, PAGE_SIZE, 0));
}

/*
 * Release a contiguous memory.
 */
void 
kgi_cfree(const void *ptr, __kgi_size_t size)
{

	contigfree((void *)ptr, size, M_KGI);
}
