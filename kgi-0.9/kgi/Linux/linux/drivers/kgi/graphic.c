/* ---------------------------------------------------------------------------
**	/dev/graphic special device file driver
** ---------------------------------------------------------------------------
**	Copyright (C)	1995-1997		Andreas Beck
**			1995-2000		Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ---------------------------------------------------------------------------
**
**	$Log: graphic.c,v $
**	Revision 1.3  2001/09/13 08:48:00  seeger_s
**	- bugfix: calculation of exec_size wrong in graph_accel_nopage()
**	
**	Revision 1.2  2001/07/03 08:52:36  seeger_s
**	- mode commands removed (will be replaced by image resource mapping)
**	
**	Revision 1.1.1.1  2000/04/18 08:50:48  seeger_s
**	- initial import of pre-SourceForge tree
**	
** 	- vm_offset changed to vm_pgoff in 2.3.25
**	- MAP_NR / virt_to_page: Argument type changed in 2.4.0-test6-pre8
**	- Other changes at unknown version levels, grep for "What version"
**	  and fix to correct version if you know.
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#undef	DEBUG
#define	DEBUG_LEVEL	1

#include <linux/version.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <asm/io.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
#include <linux/highmem.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
#define private_data private_data
#define vm_private_data vm_private_data
#define NOPAGE_SIGBUS_RETVAL NOPAGE_SIGBUS
#else
#define private_data private
#define vm_private_data vm_private
#define NOPAGE_SIGBUS_RETVAL 1
#define VM_KGIUNMAP 0
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
typedef struct wait_queue *wait_queue_head_t;
#define	init_waitqueue_head(head)	*((wait_queue_head_t *)(head)) = NULL
#define	set_current_state(a)		current->state = (a)
#endif

#include <linux/kgii.h>
#include <kgi/cmd.h>
#include <kgi/graphic.h>

#include <asm/uaccess.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
#include <asm/pgalloc.h>
#include <asm/semaphore.h>
#endif
#include <linux/poll.h>
#include <linux/wrapper.h>

#define	GRAPH_MAX_IO_BUF_SIZE	PAGE_SIZE
#define	GRAPH_MAX_NR_DEVICES	64
#define	GRAPH_MAX_NR_IMAGES	64

/*	per-device static global data.
*/
static struct
{
	unsigned int	cnt;
	graph_device_t	*ptr;
	pid_t		pid;
	gid_t		gid;

} graph_dev[GRAPH_MAX_NR_DEVICES];

/* ----------------------------------------------------------------------------
**	helper functions
** ----------------------------------------------------------------------------
*/
static inline int graph_device_id(kdev_t device)
{
	if (MAJOR(device) == GRAPH_MAJOR) {

		if (MINOR(device)) {

			return MINOR(device) - 1;
		}

		if (current && current->tty && 
			(MAJOR(current->tty->device) == TTY_MAJOR)) {

			return MINOR(current->tty->device) - 1;
		}
	}

	KRN_DEBUG(1, "Don't know how to map %x to graphic minor.",
		device);
	return -1;
}

/* NOTE	The fast_remap_page_range() and fast_unmap_page_range() should
** NOTE	move to the linux memory mapper code. Don't know if they are i386
** NOTE	specific, so we just make sure.
*/
#ifdef __i386__
/*	The following code has been rewritten from scratch using the Linux
**	kernel (asm/pgtable.h in particular) as a model. This is just to
**	handle memory mapped I/O regions where no pages need to be allocated.
**	when a fault occurs but several mappings need to be changed.
*/

#include <asm/pgtable.h>
	
static inline void fast_remap_page_range(struct mm_struct *mm,
	unsigned long addr, unsigned long end, unsigned long phys_dest,
	unsigned int vm_prot)
{
	pgprot_t prot = protection_map[vm_prot & 0xF];
	pte_t pte_entry = pte_mkold(mk_pte_phys(phys_dest, prot));
	pte_t *ptr;
	unsigned long next_pmd, beg = addr;

	/* ???	really required here? May be omitted for non-DMA buffers?
	*/
	flush_cache_range(mm, beg, end);

	next_pmd = addr & PMD_MASK;
	while(addr < end) {

		ptr = pte_offset(pmd_offset(pgd_offset(mm,addr),addr),addr);
		next_pmd += PMD_SIZE;

		if (next_pmd > end) {

			next_pmd = end;
		}

		while(addr < next_pmd) {

			*ptr++ = pte_entry;
			pte_val(pte_entry) += PAGE_SIZE;
			addr += PAGE_SIZE;
		}
	}
	/* #ifdef CONFIG_SMP
	   smp_flush_tlb();
	   #else */
	flush_tlb_range(mm, beg, end);
	/* #endif */
}

/*	unmap a range of pages in the mapping table <mm> that are mapped
**	from logical address <addr> to logical address <end>.
*/
/* ???	Can zap_page_range() be used here?
*/
static inline void fast_unmap_page_range(struct mm_struct *mm,
	unsigned long addr, unsigned long end)
{
	pte_t *ptr;
	unsigned long next_pmd, beg = addr;

	/* ???	really required here? May be ommited for non-DMA buffers?
	*/ 
	flush_cache_range(mm, beg, end);

	next_pmd = addr & PMD_MASK;
	while(addr < end) {

		ptr = pte_offset(pmd_offset(pgd_offset(mm,addr),addr),addr);
		next_pmd += PMD_SIZE;

		if (next_pmd > end) {

			next_pmd = end;
		}

		while(addr < next_pmd) {

			pte_clear(ptr++);
			addr += PAGE_SIZE;
		}
	}

	flush_tlb_range(mm, beg, end);
}

#endif

#define	VM(x)			(vma->vm_ ## x)


/*
**	resource mappings - common stuff
*/
static void graph_add_mapping(graph_file_t *file, graph_mapping_t *map)
{
	unsigned int i;

	KRN_ASSERT(file);
	KRN_ASSERT(file->device);
	KRN_ASSERT(file->device->kgi.mode);
	KRN_ASSERT(map->resource);

	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {

		if (file->device->kgi.mode->resource[i] == map->resource) {

			break;
		}
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

		map->other = map->device->mappings[i]->next;
		map->device->mappings[i]->other = map;

	} else {

		map->other = map;
		map->device->mappings[i] = map;
	}
	KRN_DEBUG(1, "added mapping %p", map);
}

static void graph_delete_mapping(graph_mapping_t *map)
{
	graph_mapping_t *prev;
	unsigned int i;

	KRN_DEBUG(1, "deleting mapping %p", map);
	KRN_ASSERT(map);
	KRN_ASSERT(map->file);
	KRN_ASSERT(map->device);

	/*	delete mapping from mappings-for-same-file list
	*/
	prev = map->next;
	while (prev->next != map) {

		prev = prev->next;
	}
	if (map->file->mappings == map) {

		map->file->mappings = (prev != map) ? prev : NULL;
	}
	prev->next = map->next;

	/*	delete from mappings-for-same-region list
	*/
	prev = map->other;
	while (prev->other != map) {

		prev = prev->other;
	}
	i = 0;
	while ((i < __KGI_MAX_NR_RESOURCES) &&
		(map->device->kgi.mode->resource[i] != map->resource)) {

		i++;
	}
	KRN_ASSERT(i < __KGI_MAX_NR_RESOURCES);

	if (map->device->mappings[i] == map) {

		map->device->mappings[i] = (prev != map) ? prev : NULL;
	}
	prev->other = map->other;

	map->file->refcnt--;

	KRN_TRACE(1, memset(map, 0, sizeof(*map)));
	kfree(map);
}

static inline void graph_unmap_mappings(graph_mapping_t *map)
{
	graph_mapping_t *first = map;

	do {
		KRN_ASSERT(map->resource == first->resource);
		KRN_DEBUG(1, "unmapping vma %p %lx-%lx", map->vma,
			map->mstart, map->mend);

		if (map->mstart) {

			fast_unmap_page_range(map->vma->vm_mm,
				map->mstart, map->mend);
			map->mstart = map->mend = 0;
		}
		map = map->other;

	} while (map != first);

	KRN_DEBUG(1, "mappings for region '%s' unmapped", map->resource->name);
}

static void graph_mapping_open(struct vm_area_struct *vma)
{
	graph_mapping_t *map = (graph_mapping_t *) VM(private_data);

	if (map) {

		if (vma != map->vma) {

			KRN_DEBUG(1, "invalidating cloned vma %p", vma);
			VM(private_data) = NULL;
		}
	}

	KRN_ASSERT(graph_device_id(VM(file)->f_dentry->d_inode->i_rdev) >= 0);
	graph_dev[graph_device_id(VM(file)->f_dentry->d_inode->i_rdev)].cnt++;
}

static void graph_mapping_close(struct vm_area_struct *vma)
{
	KRN_ASSERT(graph_device_id(VM(file)->f_dentry->d_inode->i_rdev) >= 0);
	graph_dev[graph_device_id(VM(file)->f_dentry->d_inode->i_rdev)].cnt--;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version?  Should this be done for all versions? */
	atomic_dec(&VM(file)->f_count);
#else
	VM(file)->f_count--;
#endif
}


/* -----------------------------------------------------------------------------
**	MMIO resource mapping
** -----------------------------------------------------------------------------
*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
static struct page *graph_mmio_nopage(struct vm_area_struct *vma,
	unsigned long addr, int write)
#else
static unsigned long graph_mmio_nopage(struct vm_area_struct *vma,
	unsigned long addr, int write)
#endif
{
	graph_mmio_mapping_t *map;
	kgi_mmio_region_t *mmio;

	map = (graph_mmio_mapping_t *) VM(private_data);
	if (map == NULL) {

		KRN_DEBUG(1, "invalid (cloned ?) vma %p, sending signal", vma);
		return NOPAGE_SIGBUS_RETVAL;
	}
	KRN_ASSERT(map->vma == vma);

	mmio = (kgi_mmio_region_t *) map->resource;
	KRN_ASSERT((mmio->type & KGI_RT_MASK) == KGI_RT_MMIO);
	KRN_ASSERT((VM(start) <= addr) && (addr < VM(end)));

	KRN_DEBUG(1, "mmio_nopage @%.8lx, vma %p, %x", addr, vma, write);

	if (map->device->kgi.flags & KGI_DF_FOCUSED) {

		register unsigned long pstart, mstart, mend, offset;

		current->min_flt++;

		switch(map->type) {

		case GRAPH_MM_LINEAR_LINEAR:
			KRN_DEBUG(1, "linear mapped linear mmio");
			map->offset = offset = 0;
			mstart = VM(start);
			mend   = VM(end);
			pstart = 0;
			break;

		case GRAPH_MM_LINEAR_PAGED:
			KRN_DEBUG(1, "linear mapped paged mmio");
			offset  = addr - VM(start);
			offset -= offset % mmio->win.size;
			map->offset = offset;
			mstart  = VM(start) + offset;
			mend	= mstart + mmio->win.size;
			pstart  = 0;
			KRN_ASSERT(mend <= VM(end));
			break;

		case GRAPH_MM_PAGED_LINEAR:
			KRN_DEBUG(1, "paged mapped linear mmio");
			mstart = VM(start);
			mend   = VM(end);
			offset = 0;
			pstart = map->offset;
			break;

		case GRAPH_MM_PAGED_PAGED:
			KRN_DEBUG(1, "paged mapped paged mmio");
			mstart = VM(start);
			mend   = VM(end);
			offset = map->offset;
			pstart = 0;
			break;

		default:
			KRN_INTERNAL_ERROR;
			return NOPAGE_SIGBUS_RETVAL;
		}

		KRN_DEBUG(1, "mmio_nopage @%.8lx, remap vma %p, %.8lx-%.8lx "
			"to '%s' phys %.8lx, offset %lx",
			addr, map->vma, mstart, mend,
			mmio->name, mmio->win.phys + pstart, offset);

		if (offset != mmio->offset) {

			graph_unmap_mappings((graph_mapping_t *) map);

			if (mmio->SetOffset) {

				mmio->SetOffset(mmio, offset);

			} else {

				mmio->offset = offset;
			}
		}
		fast_remap_page_range(map->vma->vm_mm, (map->mstart = mstart),
			(map->mend = mend), mmio->win.phys + pstart, map->prot);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
		/* What version? */
		KRN_DEBUG(1, "mmio_nopage() done ; 0");
		return virt_to_page(mend); /* XXX */
#else
		KRN_DEBUG(1, "mmio_nopage() done");
		return 0;
#endif

	} else {

		/* !!!	do the neccessary background actions here.
		*/
		KRN_DEBUG(1, "mmio_nopage() in background!");
		return NOPAGE_SIGBUS_RETVAL;
	}
}

/*	graph_mmio_unmap() has to unmap a mmio mapping. As we do not allow
**	partial mapping of MMIO regions, we consequently have to deny partial
**	unmapping. This is ensured in mm/mmap.c. We only have to remove our
**	local mapping state info because the page tables are fixed in mm/mmap.c.
*/
static void graph_mmio_unmap(struct vm_area_struct *vma, unsigned long start,
	size_t len)
{
	KRN_ASSERT(VM(start) == start);
	KRN_ASSERT(VM(end) == start + len);

	graph_delete_mapping((graph_mapping_t *) VM(private_data));
	VM(private_data) = NULL;
}

static struct vm_operations_struct graph_mmio_vmops =
{
	open:	graph_mapping_open,
	close:	graph_mapping_close,
	unmap:	graph_mmio_unmap,
	nopage: graph_mmio_nopage

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	/* What version? */
	/* Does GCC guarantee this w/o these lines? */
	,
	protect: NULL,
	sync:    NULL,
	advise:  NULL,
	wppage:  NULL,
	swapout: NULL,
	swapin:  NULL
#endif

};

static int graph_mmio_domap(graph_file_t *file, kgi_mmio_region_t *mmio,
	struct vm_area_struct *vma)
{
	graph_mmio_mapping_t *map;
	unsigned long size = VM(end) - VM(start);

	KRN_DEBUG(1, "mapping mmio '%s' to vma %p", mmio->name, vma);

	KRN_ASSERT(0 == (mmio->size % mmio->win.size));
	KRN_ASSERT(mmio->size >= mmio->win.size);

	if ((mmio->win.size & ~PAGE_MASK) || (size > mmio->size) ||
		((mmio->win.size == mmio->size) && (mmio->size % size)) ||
		((mmio->win.size != mmio->size) && (size < mmio->size) &&
		 (mmio->win.size != size))) {

		KRN_DEBUG(1, "mmap size %i, but region size %i, win.size %i",
			size, mmio->size, mmio->win.size);
		return -EINVAL;
	}
	KRN_DEBUG(1, "mmap size %i, region size %i, win.size %i OK",
		size, mmio->size, mmio->win.size);
	if (NULL == (map = kmalloc(sizeof(*map), GFP_KERNEL))) {

		KRN_DEBUG(1, "failed to allocate mmio mapping");
		return -ENOMEM;
	}
	memset(map, 0, sizeof(*map));

#	warning determine protection flags here!

	map->prot = VM_READ | VM_WRITE | VM_SHARED;
	map->type = (mmio->win.size == mmio->size) /* linear region? */
		? ((size == mmio->size)
			? GRAPH_MM_LINEAR_LINEAR : GRAPH_MM_PAGED_LINEAR)
		: ((size == mmio->size)
			? GRAPH_MM_LINEAR_PAGED  : GRAPH_MM_PAGED_PAGED);

	map->resource = (kgi_resource_t *) mmio;
	map->vma = vma;

	VM(private_data) = map;
	VM(ops) = &graph_mmio_vmops;
	VM(flags) |= VM_KGIUNMAP;

	graph_add_mapping(file, (graph_mapping_t *) map);

	fast_unmap_page_range(VM(mm), VM(start), VM(end));

	return KGI_EOK;
}

/* -----------------------------------------------------------------------------
**	Accelerator resource mappings
** -----------------------------------------------------------------------------
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	/* What version? */
static struct page *graph_accel_nopage(struct vm_area_struct *vma, 
	unsigned long addr, int write)
#else
static unsigned long graph_accel_nopage(struct vm_area_struct *vma, 
	unsigned long addr, int write)
#endif

{
	graph_accel_mapping_t *map;
	kgi_accel_t *accel;
	unsigned long offset = addr - VM(start);
	unsigned long nstart, nend;

	map = (graph_accel_mapping_t *) VM(private_data);
	if (map == NULL) {

		KRN_DEBUG(1, "invalid (cloned ?) vma %p, sending signal", vma);
		return NOPAGE_SIGBUS_RETVAL;
	}
	KRN_ASSERT(vma == map->vma);

	accel = (kgi_accel_t *) map->resource;
	KRN_ASSERT(accel->type == KGI_RT_ACCELERATOR);

	KRN_DEBUG(1, "accel_nopage @%.8lx, vma %p, %x", addr, vma, write);

	nstart = (map->buffer + map->buf_size) & map->buf_mask;
	nend = nstart + map->buf_size;
	if (! ((nstart <= offset) && (offset < nend))) {

		KRN_DEBUG(1, "offset %.8lx not in %.8lx-%.8lx for next buffer",
			offset, nstart, nend);
		return NOPAGE_SIGBUS_RETVAL;
	}

	if (map->buf->next->exec_state != KGI_AS_IDLE) {

		KRN_DEBUG(1, "next buffer is not IDLE, stalling");
		sleep_on(map->buf->next->executed);
		KRN_ASSERT(map->buf->next->exec_state == KGI_AS_IDLE);
	}

	if (map->mstart) {

		KRN_ASSERT(map->buf->exec_state == KGI_AS_FILL);
		KRN_DEBUG(1, "unmap & exec buffer %.8lx, %.8lx-%.8lx",
			map->buf->aperture.phys, map->mstart, map->mend);
		fast_unmap_page_range(map->vma->vm_mm, map->mstart, map->mend);
		map->buf->exec_size = 
			(offset == nstart) ? map->buf_size : (offset - nstart);
		accel->Exec(accel, map->buf);
	}

	map->buf = map->buf->next;
	map->buf->exec_state = KGI_AS_FILL;
	map->buffer = nstart;
	map->mstart = VM(start) + nstart;
	map->mend = VM(start) + nend;
	fast_remap_page_range(map->vma->vm_mm, map->mstart, map->mend, 
		map->buf->aperture.phys, VM_SHARED | VM_WRITE);

	KRN_DEBUG(1, "mapped buffer %.8lx to vma %p, %.8lx-%.8lx",
		map->buf->aperture.phys, map->vma, map->mstart, map->mend);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
	return virt_to_page(map->mend); /* XXX ? */
#else
	return 0;
#endif

}


static void graph_accel_unmap(struct vm_area_struct *vma, unsigned long start,
	size_t len)
{
	graph_accel_mapping_t *map = (graph_accel_mapping_t *) VM(private_data);
	kgi_accel_t *accel = (kgi_accel_t *) map->resource;
	kgi_accel_buffer_t *buf;

	KRN_ASSERT(VM(start) == start);
	KRN_ASSERT(VM(end) == start + len);

	map = (graph_accel_mapping_t *) VM(private_data);
	if (map == NULL) {

		return;
	}
	KRN_ASSERT(map->vma == vma);

	accel = (kgi_accel_t *) map->resource;
	KRN_ASSERT(accel->type == KGI_RT_ACCELERATOR);

	KRN_DEBUG(1, "accel_unmap vma %p, map %p", vma, map);

	buf = map->buf->next;
	while ((buf->exec == NULL) && (buf != map->buf)) {

		buf = buf->next;
	}
	if (buf->exec) {

		KRN_DEBUG(1, "waiting for accelerator to complete");
		sleep_on(buf->executed);
	}


	if (map->mstart) {

		fast_unmap_page_range(map->vma->vm_mm, map->mstart, map->mend);
		KRN_TRACE(1, map->buf->exec_state = KGI_AS_IDLE);
	}

	accel->Done(accel, buf->exec_ctx);

#	if DEBUG_LEVEL > 0
	{
		kgi_accel_buffer_t *foo = map->buf->next;

		while (foo->next != map->buf) {

			KRN_ASSERT(foo->exec_state == KGI_AS_IDLE);
			KRN_ASSERT(foo->exec == NULL);
			KRN_ASSERT(accel->ctx != foo->exec_ctx);
			foo = foo->next;
		}
	}
#	endif

	/*
	**	free context, buffers and buffer infos
	*/
	buf = map->buf;
	kfree(buf->exec_ctx);

	KRN_ASSERT(buf);
	KRN_ASSERT(buf->next);
	KRN_ASSERT(buf->next != map->buf);
	do {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
/* What version? */
		struct page *page;
		kgi_accel_buffer_t *next = buf->next;

		for (page = virt_to_page(buf->aperture.virt); page <= virt_to_page(buf->aperture.virt + map->buf_size); page++)
			mem_map_unreserve(page);
#else
		unsigned long mstart = MAP_NR(buf->aperture.phys);
		unsigned long mend = MAP_NR(buf->aperture.phys + map->buf_size);
		unsigned int i;
		kgi_accel_buffer_t *next = buf->next;


		for (i = mstart; i < mend; i++) {
			mem_map_unreserve(i);
		}
#endif

		KRN_DEBUG(1, "freeing %.8lx with order %i",
			buf->aperture.virt, map->buf_order);
		free_pages((unsigned long) buf->aperture.virt, map->buf_order);
		kfree(buf->executed);
		kfree(buf);

		buf = next;

	} while (buf != map->buf);
	KRN_TRACE(1, map->buf = NULL);

	graph_delete_mapping((graph_mapping_t *) map);
	VM(private_data) = NULL;
}


static struct vm_operations_struct graph_accel_vmops =
{
	open:	graph_mapping_open,
	close:	graph_mapping_close,
	unmap:	graph_accel_unmap,
	nopage:	graph_accel_nopage

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	/* What version? */
	/* Does GCC guarantee this w/o these lines? */
	,
	protect: NULL,
	sync:    NULL,
	advise:  NULL,
	wppage:  NULL,
	swapout: NULL,
	swapin:  NULL
#endif

};


static int graph_accel_domap(graph_file_t *file, kgi_accel_t *accel, 
	struct vm_area_struct *vma)
{
	graph_accel_mapping_t *map = NULL;
	unsigned long min_order, max_order, priority, order, buffers, i;
	kgi_accel_buffer_t *buf[16];
	kgi_accel_context_t *context = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
	min_order = (VM(pgoff) & GRAPH_MMAP_ACCEL_MINSIZE) >>
		GRAPH_MMAP_ACCEL_MINSIZE_SHIFT;
	order = max_order = (VM(pgoff) & GRAPH_MMAP_ACCEL_MAXSIZE) >>
		GRAPH_MMAP_ACCEL_MAXSIZE_SHIFT;
	buffers = 1 << ((VM(pgoff) & GRAPH_MMAP_ACCEL_BUFFERS) >>
		GRAPH_MMAP_ACCEL_BUFFERS_SHIFT);
	priority = (VM(pgoff) & GRAPH_MMAP_ACCEL_PRIORITY) >>
		GRAPH_MMAP_ACCEL_PRIORITY_SHIFT;
#else
	min_order = (VM(offset) & GRAPH_MMAP_ACCEL_MINSIZE) >>
		GRAPH_MMAP_ACCEL_MINSIZE_SHIFT;
	order = max_order = (VM(offset) & GRAPH_MMAP_ACCEL_MAXSIZE) >>
		GRAPH_MMAP_ACCEL_MAXSIZE_SHIFT;
	buffers = 1 << ((VM(offset) & GRAPH_MMAP_ACCEL_BUFFERS) >>
		GRAPH_MMAP_ACCEL_BUFFERS_SHIFT);
	priority = (VM(offset) & GRAPH_MMAP_ACCEL_PRIORITY) >>
		GRAPH_MMAP_ACCEL_PRIORITY_SHIFT;
#endif
	memset(buf, 0, sizeof(buf));

	if (buffers > sizeof(buf)/sizeof(buf[0])) {

		KRN_DEBUG(1, "too many buffers (%i) requested", buffers);
		return -EINVAL;
	}

	if (min_order > max_order) {

		KRN_DEBUG(1, "invalid buffer size range (min %i, max %i)",
 			min_order, max_order);
		return -EINVAL;
	}

	/* !!!	check for process limits (don't allow allocation of too
	** !!!	much memory).
	*/

	/*	allocate mapping and init accelerator context
	*/
	if (! (map = kmalloc(sizeof(*map), GFP_KERNEL))) {

		KRN_DEBUG(1, "failed to allocate accelerator mapping");
		goto no_memory;
	}
	memset(map, 0, sizeof(*map));
	if (! (context = kmalloc(accel->ctx_size, GFP_KERNEL))) {

		KRN_DEBUG(1, "failed to allocate accelerator context");
		goto no_memory; 
	}
	memset(context, 0, accel->ctx_size);
	context->aperture.size = accel->ctx_size;
	context->aperture.virt = (void *) context;
	context->aperture.bus  = virt_to_bus(context);
	context->aperture.phys = virt_to_phys(context);
	accel->Init(accel, context);

	/*	allocate buffer info
	*/
	KRN_TRACE(1, memset(buf, 0, sizeof(buf)));
	for (i = 0; i < buffers; i++) {

		buf[i] = kmalloc(sizeof(kgi_accel_buffer_t), GFP_KERNEL);
		if (buf[i]) {

			memset(buf[i], 0, sizeof(*buf[i]));
			continue;
		}

		KRN_DEBUG(1, "failed to allocate buffer infos");
		goto no_memory;
	}

	/*	allocate DMA buffers. These must be contigous in physical
	**	address space and all of same size. We try beginning with
	**	max_order and check for smaller ones if this fails.
	*/
	do {
		for (i = 0; buf[i]->aperture.virt && (i < buffers); i++) {

			KRN_DEBUG(1, "freeing %p with order %i",
				buf[i]->aperture.virt, order + 1);
			free_pages((unsigned long) buf[i]->aperture.virt,
				order + 1);
		}

		KRN_DEBUG(1, "trying %i byte buffers", 1 << (order+PAGE_SHIFT));
		for (i = 0; i < buffers; i++) {

			buf[i]->aperture.virt = (void *)
				__get_free_pages(GFP_KERNEL, order);
			if (! buf[i]->aperture.virt) {

				break;
			}
			KRN_DEBUG(1, "allocated %.8lx with order %i",
				buf[i]->aperture.virt, order);
			buf[i]->executed = kmalloc(sizeof(wait_queue_head_t),
				GFP_KERNEL);
			if (! buf[i]->executed) {

				break;
			}
		}

	} while ((i != buffers) && (--order >= min_order));
	if (i != buffers) {

		KRN_DEBUG(1, "failed to allocate DMA buffers");
		goto no_memory;
	}

	/*	arrange buffers in circular list and attach to mapping.
	**	Finally attach mapping to vma.
	*/
	for (i = 0; i < buffers; i++) {

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	  /* What version? */
		struct page *page;
#else
		unsigned long mstart, mend, m;

#endif
		buf[i]->next = buf[((i + 1) < buffers) ? (i + 1) : 0];
		buf[i]->exec = NULL;

		buf[i]->exec_pri = priority;
		buf[i]->exec_ctx = context;
		buf[i]->exec_state = KGI_AS_IDLE;

		buf[i]->aperture.size = 1 << (order + PAGE_SHIFT);
		buf[i]->aperture.phys = virt_to_phys(buf[i]->aperture.virt);
		buf[i]->aperture.bus  = virt_to_bus(buf[i]->aperture.virt);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
		/* What version? */
		for (page = virt_to_page(buf[i]->aperture.virt); page <= virt_to_page(buf[i]->aperture.virt + buf[i]->aperture.size); page++) {
			mem_map_reserve(page);
		}
#else
		mstart = MAP_NR(buf[i]->aperture.phys);
		mend   = MAP_NR(buf[i]->aperture.phys + buf[i]->aperture.size);
		for (m = mstart; m < mend; m++) {

			mem_map_reserve(m);
		}

#endif
		init_waitqueue_head(buf[i]->executed);
	}

	map->buf = buf[0];
	map->buf_order = order;
	map->buf_size = 1 << (order + PAGE_SHIFT);
	map->buf_mask = (buffers - 1) << (order + PAGE_SHIFT);

	map->resource = (kgi_resource_t *) accel;
	map->vma = vma;

	VM(private_data) = map;
	VM(ops) = &graph_accel_vmops;
	VM(flags) |= VM_KGIUNMAP;

	graph_add_mapping(file, (graph_mapping_t *) map);

	map->buf->exec_state = KGI_AS_FILL;
	fast_unmap_page_range(VM(mm), VM(start), VM(end));
	map->mstart = VM(start);
	map->mend   = VM(start) + map->buf_size;
	fast_remap_page_range(map->vma->vm_mm, map->mstart, map->mend,
		map->buf->aperture.phys, VM_SHARED | VM_WRITE);
	KRN_DEBUG(1, "mapped buffer %.8lx to vma %p, %.8lx-%.8lx",
		map->buf->aperture.phys, map->vma, map->mstart, map->mend);

	return KGI_EOK;

no_memory:
	KRN_DEBUG(1, "no memory");
	kfree(map);
	if (context) {

		accel->Done(accel, context);
		kfree(context);
	}

	order++;
	for (i = 0; buf[i] && (i < buffers); i++) {

		if (buf[i]->executed) {

			kfree(buf[i]->executed);
		}
		if (buf[i]->aperture.virt) {

			KRN_DEBUG(1, "freeing %.8lx with order %i",
				buf[i]->aperture.virt, order);
			free_pages((unsigned long) buf[i]->aperture.virt,
				order);
		}
		kfree(buf[i]);
	}
	return -ENOMEM;
}


/*	graph_mmap() does the neccessary stuff to add a mapping to a file
**	We assume:
**	* one vm_area_struct per mapping
**	* vm_area_structs served by VM_FASTHANDLER are not merged together
*/
#undef private_data
static int graph_mmap(struct file *kfile, struct vm_area_struct *vma)
{
	int err;
	graph_file_t *file = (graph_file_t *) kfile->private_data;
	const kgi_resource_t *resource = NULL;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
	if (GRAPH_MMAP_RESOURCE(VM(pgoff)) < __KGI_MAX_NR_RESOURCES) {
#else
	if (GRAPH_MMAP_RESOURCE(VM(offset)) < __KGI_MAX_NR_RESOURCES) {
#endif

		resource = file->device->kgi.mode ?
			file->device->kgi.mode->resource[
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
			GRAPH_MMAP_RESOURCE(VM(pgoff))] : NULL;
#else
			GRAPH_MMAP_RESOURCE(VM(offset))] : NULL;
#endif
	}
	if (resource == NULL) {

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
		KRN_DEBUG(1, "invalid resource %i specified! (mode == %p)",
			GRAPH_MMAP_RESOURCE(VM(pgoff)), file->device->kgi.mode);
#else
		KRN_DEBUG(1, "invalid resource %i specified! (mode == %p)",
			GRAPH_MMAP_RESOURCE(VM(offset)), file->device->kgi.mode);
#endif
		return -ENXIO;
	}


	/*
	** !!!	do protection handling/checking!
	*/


	if ((err = zeromap_page_range(VM(start), VM(end) - VM(start), 
		protection_map[VM_SHARED | VM_READ]))) {

		KRN_DEBUG(1, "zeromap_page_range() failed");
		return err;
	}

	switch (resource->type & KGI_RT_MASK) {

	case KGI_RT_MMIO:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
		err = (GRAPH_MMAP_TYPE(VM(pgoff)) != GRAPH_MMAP_TYPE_MMIO)
			? -EINVAL : KGI_EOK;
#else
		err = (GRAPH_MMAP_TYPE(VM(offset)) != GRAPH_MMAP_TYPE_MMIO)
			? -EINVAL : KGI_EOK;
#endif
		if (err || (err = graph_mmio_domap(file, (kgi_mmio_region_t *)
			resource, vma))) {

			KRN_DEBUG(1, "mmio mapping failed");
			return err;
		}
		KRN_DEBUG(1, "mmio mapping succeeded");
		break;

	case KGI_RT_ACCEL:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
		err = (GRAPH_MMAP_TYPE(VM(pgoff)) != GRAPH_MMAP_TYPE_ACCEL)
			? -EINVAL : KGI_EOK;
#else
		err = (GRAPH_MMAP_TYPE(VM(offset)) != GRAPH_MMAP_TYPE_ACCEL)
			? -EINVAL : KGI_EOK;
#endif
		KRN_ASSERT(err == KGI_EOK);
		if (err || (err = graph_accel_domap(file, (kgi_accel_t *)
			resource, vma))) {

			KRN_DEBUG(1, "accel mapping failed");
			return err;
		}
		break;

	default:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,25)
		KRN_DEBUG(1, "unkown mapping type %.8x", 
			GRAPH_MMAP_TYPE(VM(pgoff)));
#else
		KRN_DEBUG(1, "unkown mapping type %.8x", 
			GRAPH_MMAP_TYPE(VM(offset)));
#endif
		return -ENXIO;
	}
	/*	We do inode reference counting in graph_mapping_{open,close}.
	*/
	KRN_ASSERT(VM(ops)->open == graph_mapping_open);
	KRN_ASSERT(VM(ops)->close == graph_mapping_close);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        /* What version?  All versions? */
	atomic_add(1,&kfile->f_count);
#else
	kfile->f_count++;
#endif
	VM(file) = kfile;
	VM(flags) |= VM_FASTHANDLER;
	VM(ops)->open(vma);

	KRN_DEBUG(1, "mapped resource %s @ %.8x, size %u",
		resource->name, VM(start), VM(end) - VM(start));
	return KGI_EOK;
}

#define graph_device_kgi_map	NULL
#define	graph_device_kgi_unmap	NULL

/*
**	command resource
*/
static inline int graph_command(graph_file_t *file, unsigned int cmd, 
	void *in_buffer, void **out_buffer, unsigned long *out_size)
{
	switch (cmd) {

	case KGIC_MAPPER_IDENTIFY:
		{	/*	For now only force client identification and
			**	identify myself (no compatibility checks).
			*/
			kgic_mapper_identify_result_t	*out = *out_buffer;
			KRN_ASSERT(*out_size >= sizeof(*out));
			*out_size = sizeof(*out);

			if (file->flags & GRAPH_FF_CLIENT_IDENTIFIED) {

				return -EPROTO;
			}

			file->flags |= GRAPH_FF_CLIENT_IDENTIFIED;

			memset(out->mapper, 0, sizeof(out->mapper));
			strncpy(out->mapper, GRAPH_NAME, sizeof(out->mapper));
			out->mapper_version.major = 0;
			out->mapper_version.minor = 9;
			out->mapper_version.patch = 0;
			out->mapper_version.extra = 0;
			out->resources = __KGI_MAX_NR_RESOURCES;
#warning tell client if it is session leader or not.

			return KGI_EOK;
		}
	}

	/*	all commands below require identification
	*/
	if (! (file->flags & GRAPH_FF_CLIENT_IDENTIFIED)) {

		KRN_DEBUG(1, "cmd = %.8x, but client has not yet identified",
			cmd);
		return -EPROTO;
	}
	switch (cmd) {

	case KGIC_MAPPER_SET_IMAGES:
		{	/*	Only the session leader may set the number of
			**	images as long as device isn't registered.
			*/
			kgic_mapper_set_images_request_t *in  = in_buffer;
			kgi_size_t size;

			if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
				KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1,
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "mode already checked"
					: "client is not session leader");
				return -EPROTO;
			}
#warning what about resources still mapped?!
			if (file->device->kgi.mode) {

				kfree(file->device->kgi.mode->dev_mode);
				file->device->kgi.mode->dev_mode = NULL;
				kfree(file->device->kgi.mode);
				file->device->kgi.mode = NULL;
			}

			if ((in->images < 1) ||
				(GRAPH_MAX_NR_IMAGES < in->images)) {

				return -ENOMEM;
			}
			size = sizeof(kgi_mode_t) + 
				sizeof(kgi_image_mode_t)*(in->images - 1);
			file->device->kgi.mode = kmalloc(size, GFP_KERNEL);
			if (NULL == file->device->kgi.mode) {

				return -ENOMEM;
			}
			memset(file->device->kgi.mode, 0, size);
			file->device->kgi.mode->revision = KGI_MODE_REVISION;
			file->device->kgi.mode->images = in->images;
			return KGI_EOK;
		}

	case KGIC_MAPPER_GET_IMAGES:
		{
			kgic_mapper_get_images_result_t *out = *out_buffer;
			*out_size = sizeof(*out);

			out->images = file->device->kgi.mode 
				? file->device->kgi.mode->images : 0;
			return KGI_EOK;
		}

	case KGIC_MAPPER_SET_IMAGE_MODE:
		{
			kgic_mapper_set_image_mode_request_t *in = in_buffer;

			if ((!(file->flags & GRAPH_FF_SESSION_LEADER)) ||
				KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1,
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "mode already checked"
					: "client is not session leader");
				return -EPROTO;
			}

			if (file->device->kgi.mode->images <= in->image) {

				return -EINVAL;
			}

			file->device->kgi.mode->img[in->image] =
				in->mode;

			return KGI_EOK;
		}

	case KGIC_MAPPER_GET_IMAGE_MODE:
		{
			kgic_mapper_get_image_mode_request_t *in = in_buffer;
			kgic_mapper_get_image_mode_result_t *out = *out_buffer;
			*out_size = sizeof(*out);

			if (NULL == file->device->kgi.mode) {

				KRN_DEBUG(1, "number of images not yet set");
				return -EPROTO;
			}
			if (file->device->kgi.mode->images <= in->image) {

				return -EINVAL;
			}
			out->mode = file->device->kgi.mode->img[in->image];
			return KGI_EOK;
		}

	case KGIC_MAPPER_MODE_CHECK:
		{
			kgi_u_t i;
			kgi_error_t err;
			if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
				(NULL == file->device->kgi.mode)) {

				KRN_DEBUG(1, "%s",
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "number of images not yet set"
					: "client is not session leader");
				return -EPROTO;
			}
			KRN_ASSERT(!KGI_VALID_DEVICE_ID(file->device->kgi.id));
			KRN_ASSERT(!KGI_VALID_DISPLAY_ID(file->device->kgi.dpy_id));
			file->device->kgi.MapDevice   = graph_device_kgi_map;
			file->device->kgi.UnmapDevice = graph_device_kgi_unmap;
			file->device->kgi.HandleEvent = NULL;
/*			file->device->kgi.private	*/
			err = kgi_register_device(&(file->device->kgi),
					file->device_id);
			if (KGI_EOK != err) {

				return err;
			}
#if defined(DEBUG_LEVEL) && (DEBUG_LEVEL > 0)
			i = 0;
			while (i < __KGI_MAX_NR_RESOURCES) {

				const kgi_resource_t *r =
					file->device->kgi.mode->resource[i];

				if (NULL == r) {

					break;
				}

				KRN_DEBUG(1, "resource %i (%s) has type %.8x",
					i, r->name, r->type);
				i++;
			}
#endif
			return KGI_EOK;
		}

	case KGIC_MAPPER_MODE_SET:
		{
			kgi_device_t *prev;
			if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
				!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1, "%s",
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "mode not yet checked"
					: "client is not session leader");
				return -EPROTO;
			}
			prev = kgi_current_focus(file->device->kgi.dpy_id);
			if (prev) {

				KRN_DEBUG(1, "unmapping previous device");
				switch (kgi_unmap_device(prev->id)) {

				case KGI_EOK:
					break;
				default:
					KRN_DEBUG(1,"can't unmap current focus");
					return -EBUSY;
				}
			}
			kgi_map_device(file->device->kgi.id);
			KRN_DEBUG(1, "mapping new device");
			return KGI_EOK;
		}

	case KGIC_MAPPER_MODE_UNSET:
		{
			if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
				!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1, "%s",
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "mode not yet checked"
					: "client is not session leader");
				return -EPROTO;
			}
			kgi_unmap_device(file->device->kgi.id);
			return KGI_EOK;
		}

	case KGIC_MAPPER_MODE_DONE:
		{
			if (! (file->flags & GRAPH_FF_SESSION_LEADER) ||
				!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1, "%s",
					(file->flags & GRAPH_FF_SESSION_LEADER)
					? "mode not yet checked"
					: "client is not session leader");
				return -EPROTO;
			}
			if (file->device->kgi.flags & KGI_DF_FOCUSED) {

				kgi_unmap_device(file->device->kgi.id);
			}
			kgi_unregister_device(&(file->device->kgi));
			return KGI_EOK;
		}

	case KGIC_MAPPER_RESOURCE_INFO:
		{
			kgic_mapper_resource_info_request_t *in = in_buffer;
			kgic_mapper_resource_info_result_t *out = *out_buffer;
			*out_size = sizeof(*out);

			if (! KGI_VALID_DEVICE_ID(file->device->kgi.id)) {

				KRN_DEBUG(1, "mode not yet checked");
				return -EAGAIN;
			}
			if (__KGI_MAX_NR_RESOURCES <= in->resource) {

				KRN_DEBUG(1, "invalid resource ID");
				return -EINVAL;
			}
			if (file->device->kgi.mode->resource[in->resource]) {

				const union {

					kgi_resource_t		common;
					kgi_mmio_region_t	mmio;
					kgi_accel_t		accel;
					kgi_shmem_t		shmem;

				} *r = (void *) file->device->kgi.mode->resource[
					in->resource];

				memset(out, 0, sizeof(*out));

				strncpy(out->name, r->common.name,
					sizeof(out->name));
				out->name[sizeof(out->name)-1] = 0;
				out->resource = in->resource;
				out->type = r->common.type;
				out->protection = r->common.prot;
				switch (r->common.type & KGI_RT_MASK) {

				case KGI_RT_MMIO:
					out->info.mmio.access = r->mmio.access;
					out->info.mmio.align  = r->mmio.align;
					out->info.mmio.size   = r->mmio.size;
					out->info.mmio.window =
						r->mmio.win.size;
					break;

				case KGI_RT_ACCEL:
					out->info.accel.buffers =
						r->accel.buffers;
					out->info.accel.buffer_size =
						r->accel.buffer_size;
					break;

				case KGI_RT_SHMEM:
					out->info.shmem.aperture_size =
						r->shmem.aperture_size;
					break;

				default:
					KRN_DEBUG(0,
						"unknown resource type %.8x",
						r->common.type & KGI_RT_MASK);
				}
				return KGI_EOK;
				
			} else {

				KRN_DEBUG(1, "no such resource %i",
					in->resource);
				return -ENXIO;
			}
		}

	default:
		KRN_DEBUG(1, "command %.4x not (yet) implemented", cmd);
		return -ENXIO;
	}
}

/*	ioctl() services. This is the primary command interface. Other 
**	driver-specific interfaces may be provided using mmio-regions or 
**	by giving direct access to registers where this is safe. 
**	Other OSes might even have other means.
*/
static int graph_ioctl(struct inode *inode, struct file *kfile,
	unsigned int cmd, unsigned long arg)
{
	graph_file_t *file = (graph_file_t *) kfile->private_data;
	int io_result	= KGI_EOK;
	kgi_size_t io_size	= GRAPH_MAX_IO_BUF_SIZE;
	void *io_ibuf	= file->device->cmd_in_buffer;
	void *io_obuf	= file->device->cmd_out_buffer;

	down(&file->device->cmd_mutex);

	if (KGIC_READ(cmd)) {

		if (GRAPH_MAX_IO_BUF_SIZE < KGIC_SIZE(cmd)) {

			KRN_DEBUG(1, "buffer too small for cmd %.8x", cmd);
			io_result = -ENOMEM;
			goto unlock;
		}
		io_result = verify_area(VERIFY_READ, (void *) arg,
			KGIC_SIZE(cmd));
		if (io_result != KGI_EOK) {

			goto unlock;
	        }
		copy_from_user(io_ibuf, (void *) arg, KGIC_SIZE(cmd));

	} else {

		*((unsigned long *) io_ibuf) = arg;
	}

	switch (cmd & KGIC_TYPE_MASK) {

	case KGIC_MAPPER_COMMAND:
		io_result = graph_command(file,
			cmd, io_ibuf, &io_obuf, &io_size);
		break;

	case KGIC_DISPLAY_COMMAND:
		io_result = KGI_VALID_DEVICE_ID(file->device->kgi.id)
			? kgidev_display_command(&file->device->kgi,
				cmd, io_ibuf, &io_obuf, &io_size)
			: -EPROTO;
		break;

	default:
		KRN_DEBUG(1, "command type %.4x not (yet) implemented",
			cmd & KGIC_TYPE_MASK);
		io_result = -EINVAL;
		goto unlock;
	}

	if (KGIC_WRITE(cmd) && (io_result == KGI_EOK)) {

		io_result = verify_area(VERIFY_WRITE, (void *) arg, io_size);
		if (io_result != KGI_EOK) {

			goto unlock;
		}
	
		if (io_obuf) {

			copy_to_user((void *) arg, io_obuf, io_size);

		} else {

			clear_user((void *) arg, io_size);
		}
	}

unlock:
	up(&file->device->cmd_mutex);
	return io_result;
}

static int graph_device_init(int device_id)
{
	graph_device_t *device;

	KRN_ASSERT(device_id >= 0);

	if (graph_dev[device_id].ptr) {

		KRN_DEBUG(1, "graph_device %i already initialized", device_id);
		graph_dev[device_id].cnt++;
		return KGI_EOK;
	}
	if (graph_dev[device_id].cnt) {

		KRN_DEBUG(1, "graph_device %i has pending (mmap) references",
			device_id);
		return -EBUSY;
	}
	KRN_ASSERT(graph_dev[device_id].pid == 0);
	KRN_ASSERT(graph_dev[device_id].gid == 0);

	if (NULL == (device = kmalloc(sizeof(*device), GFP_KERNEL))) {

		KRN_DEBUG(1, "failed to allocate graph_device %i", device_id);
		return -ENOMEM;
	}
	memset(device, 0, sizeof(*device));	
	device->kgi.id = KGI_INVALID_DEVICE;
	device->kgi.dpy_id = KGI_INVALID_DISPLAY;

	device->cmd_in_buffer  = kmalloc(GRAPH_MAX_IO_BUF_SIZE, GFP_KERNEL);
	device->cmd_out_buffer = kmalloc(GRAPH_MAX_IO_BUF_SIZE, GFP_KERNEL);
	if ((NULL == device->cmd_in_buffer) ||
		(NULL == device->cmd_out_buffer)) {

		kfree(device->cmd_in_buffer);
		kfree(device->cmd_out_buffer);
		kfree(device);
		return -ENOMEM;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	/* What version? */
	init_MUTEX(&device->cmd_mutex);
#else
	device->cmd_mutex = MUTEX;
#endif

	graph_dev[device_id].pid = current->pid;
	graph_dev[device_id].gid = current->gid;
	graph_dev[device_id].ptr = device;
	graph_dev[device_id].cnt++;

	KRN_DEBUG(1, "graph_device %i initialized", device_id);
	return KGI_EOK;
}

static void graph_device_done(int device_id)
{
	KRN_ASSERT(device_id >= 0);

	if (--graph_dev[device_id].cnt) {

		KRN_DEBUG(1, "graph_device %i closed", device_id);
		return;
	}

	if (KGI_VALID_DEVICE_ID(graph_dev[device_id].ptr->kgi.id)) {

		KRN_DEBUG(1, "device still registered (id %i)",
			graph_dev[device_id].ptr->kgi.id);
		if (graph_dev[device_id].ptr->kgi.flags & KGI_DF_FOCUSED) {

			kgi_unmap_device(graph_dev[device_id].ptr->kgi.id);
		}
		kgi_unregister_device(&(graph_dev[device_id].ptr->kgi));
		kfree(graph_dev[device_id].ptr->kgi.mode->dev_mode);
		graph_dev[device_id].ptr->kgi.mode->dev_mode = NULL;
	}

	kfree(graph_dev[device_id].ptr->cmd_in_buffer);
	kfree(graph_dev[device_id].ptr->cmd_out_buffer);
	graph_dev[device_id].ptr->cmd_in_buffer = NULL;
	graph_dev[device_id].ptr->cmd_out_buffer = NULL;

	kfree(graph_dev[device_id].ptr);
	graph_dev[device_id].ptr = NULL;

	KRN_ASSERT(graph_dev[device_id].cnt == 0);
	KRN_ASSERT(graph_dev[device_id].ptr == NULL);
	KRN_ASSERT(graph_dev[device_id].pid == 0);
	KRN_ASSERT(graph_dev[device_id].gid == 0);

	KRN_DEBUG(1, "graph_device %i finally closed", device_id);
}


/*	Open a graphic special file. We force all processes that made
**	references mmap() to invalidate these before we allow any reallocation.
*/
static int graph_open(struct inode *inode, struct file *kfile)
{
	graph_file_t *file;
	int err, device_id = graph_device_id(inode->i_rdev);

	KRN_DEBUG(1, "open() graphics device %i", device_id);

	if (device_id < 0) {

		KRN_DEBUG(1, "open() failed: invalid device %x", inode->i_rdev);
		return -ENXIO;
	}

	if ((err = graph_device_init(device_id))) {
		
		return err;
	}

	if (NULL == (file = kmalloc(sizeof(*file), GFP_KERNEL))) {
	
		KRN_DEBUG(1, "failed to allocate graph_file");
		graph_device_done(device_id);
		return -ENOMEM;
	}
	memset(file, 0, sizeof(*file));
	file->refcnt++;
	file->device = graph_dev[device_id].ptr;
	file->device_id = device_id;
	if (graph_dev[device_id].pid == current->pid) {

		file->flags |= GRAPH_FF_SESSION_LEADER;
	}
	kfile->private_data = file;
	return KGI_EOK;
}

static int graph_release(struct inode *inode, struct file *kfile)
{
	graph_file_t *file;
	int device_id;

	if (! (kfile && kfile->private_data)) {

		KRN_DEBUG(1, "graph_release() on closed file?!");
		return -EINVAL;
	}
	file = (graph_file_t *) kfile->private_data;
	device_id = file->device_id;

	KRN_DEBUG(1, "closing graph device %i (refcnt %i)", 
		device_id, graph_dev[device_id].cnt);

	/* Delete all mappings for this file. The vm_area_structs remain
	** valid until the process explicitely unmaps them; if they are
	** referenced but have no valid vm_private_data field we send SIGBUS.
	*/
	while (file->mappings) {

		graph_mapping_t *map = file->mappings->next;

		KRN_ASSERT(map->vma);

		if (map->mstart) {

			fast_unmap_page_range(map->vma->vm_mm, map->mstart,
				map->mend);
		}
		map->vma->vm_private_data = NULL;
		graph_delete_mapping(map);
	}

	kfile->private_data = NULL;
	file->refcnt--;
	file->device = NULL;
	file->device_id = -1;
	KRN_ASSERT(file->refcnt == 0);
	kfree(file);

	if (current->pid == graph_dev[device_id].pid) {

		KRN_DEBUG(1, "session leader (pid %i) closed graph_device %i",
			graph_dev[device_id].pid, device_id);
		graph_dev[device_id].pid = 0;
		graph_dev[device_id].gid = 0;
	}

	graph_device_done(device_id);
	return KGI_EOK;
}

static struct file_operations graph_fops =
{
	ioctl:			graph_ioctl,
	mmap:			graph_mmap,
	open:			graph_open,
	release:		graph_release,
	/* Does GCC guarantee NULLs w/o explicit inits here? */
	flush:			NULL,
	llseek:			NULL,
	read:			NULL,
	write:			NULL,
	readdir:		NULL,
	poll:			NULL,
	fsync:			NULL,
	fasync:			NULL,
	lock:			NULL,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,0)
	check_media_change:	NULL,
	revalidate:		NULL,
#endif

};

/*
**	Kernel Interface
*/
int dev_graphic_init(void)
{
	memset(&graph_dev, 0, sizeof(graph_dev));

	if (register_chrdev(GRAPH_MAJOR, GRAPH_NAME, &graph_fops)) {

		printk(KERN_ERR "Can't register " GRAPH_NAME " devices.\n");
		return -EBUSY;
	}
	printk(KERN_NOTICE GRAPH_NAME " devices registered.\n");
	return KGI_EOK;
}

#ifdef __MODULE__
void init_module(void)
{
	return dev_graphic_init();
}

void cleanup_module(void)
{
	unregister_chrdev(GRAPH_MAJOR, GRAPH_NAME);
}
#endif
