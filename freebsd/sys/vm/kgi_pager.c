/*-
 * Copyright (C) 2002, 2004 Nicholas Souchu
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
 * FreeBSD KGI pager
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/mutex.h>
#include <sys/mman.h>
#include <sys/sx.h>

#include <sys/sysproto.h>
#include <sys/filedesc.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <sys/vmmeter.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>
#include <vm/uma.h>

#include <vm/kgi_pager.h>

static void kgi_pager_init(void);
static vm_object_t kgi_pager_alloc(void *handle, vm_ooffset_t size,
		vm_prot_t prot, vm_ooffset_t foff, struct ucred *cred);
static void kgi_pager_dealloc(vm_object_t object);
static int kgi_pager_getpages(vm_object_t object, vm_page_t *m,
		int count, int reqpage);
static void kgi_pager_putpages(vm_object_t object, vm_page_t *m, int count,
		boolean_t sync, int *rtvals);
static boolean_t kgi_pager_haspage(vm_object_t object, vm_pindex_t pindex,
		int *before, int *after);

/* list of device pager objects */
static struct pagerlst kgi_pager_object_list;
static struct vmalst kgi_pager_vma_list;

/* protect against object creation */
static struct sx kgi_pager_sx;
/* protect list manipulation */
static struct mtx kgi_pager_mtx;

static uma_zone_t fakepg_zone;
static uma_zone_t vma_zone;

static vm_area_t kgi_pager_vma_allocate(vm_ooffset_t foff, vm_ooffset_t size,
		vm_prot_t prot);
static void kgi_pager_deallocate_vma(vm_area_t vma);
static vm_page_t kgi_pager_getfake(void);
static void kgi_pager_putfake(vm_page_t m);

static void
kgi_pager_init(void)
{
	TAILQ_INIT(&kgi_pager_object_list);
	TAILQ_INIT(&kgi_pager_vma_list);
	sx_init(&kgi_pager_sx, "kgi_pager create");
	mtx_init(&kgi_pager_mtx, "kgi_pager list", NULL, MTX_DEF);
	fakepg_zone = uma_zcreate("KGIP fakepg", sizeof(struct vm_page),
				  NULL, NULL, NULL, NULL, UMA_ALIGN_PTR,
				  UMA_ZONE_NOFREE|UMA_ZONE_VM);
	vma_zone = uma_zcreate("KGIP vma", sizeof(struct vm_area_struct),
			       NULL, NULL, NULL, NULL, UMA_ALIGN_PTR,
			       UMA_ZONE_NOFREE|UMA_ZONE_VM);
	return;
}

static vm_object_t
kgi_pager_alloc(void *handle, vm_ooffset_t size, vm_prot_t prot,
		vm_ooffset_t foff, struct ucred *cred)
{
	vm_object_t object;
	vm_area_t vma;
	struct cdev *dev;
	struct cdevsw *csw;
	int err = 0;

	/*
	 * Make sure this device can be mapped.
	 */
	dev = (struct cdev *)handle;
	csw = dev_refthread(dev);
	if (csw == NULL)
		return (NULL);

	mtx_lock(&Giant);
	/*
	 * Offset should be page aligned.
	 */
	if (foff & PAGE_MASK) {
		mtx_unlock(&Giant);
		return (NULL);
	}

	size = round_page(size);

	/* Allocate a new vm area */
	vma = kgi_pager_vma_allocate(foff, size, prot);

	/* Map the area to the resource.
	 * graph_mmap should check the protection.
	 */
	err = graph_mmap(dev, vma);

	/* XXX handle error */

	/*
	 * Lock to prevent object creation race condition.
	 */
	sx_xlock(&kgi_pager_sx);

	/*
	 * Create an object for every mapping.
	 * Allocate object and associate it with the pager.
	 * The idea is to have only one reference to the object and
	 * get kgi_pager_dealloc() called for every release of a mapping.
	 * True, it duplicates the role of the object but for now
	 * this is best to keep compatibility without too much changes in
	 * the existing code.
	 */
	object = vm_object_allocate(OBJT_KGI, OFF_TO_IDX(foff + size));
	object->handle = vma;
	TAILQ_INIT(&object->un_pager.kgip.kgip_pglist);
	mtx_lock(&kgi_pager_mtx);
	TAILQ_INSERT_TAIL(&kgi_pager_object_list, object, pager_object_list);
	mtx_unlock(&kgi_pager_mtx);

	vma->vm_object = object;
	TAILQ_INSERT_TAIL(&kgi_pager_vma_list, vma, pager_vma_list);

	sx_xunlock(&kgi_pager_sx);
	mtx_unlock(&Giant);
	dev_relthread(dev);
	return (object);
}

/* 
 * The object shall be locked.
 */
static void
kgi_pager_dealloc(vm_object_t object)
{
	vm_area_t vma = (vm_area_t )object->handle;
	vm_page_t m;

	mtx_lock(&kgi_pager_mtx);
	TAILQ_REMOVE(&kgi_pager_object_list, object, pager_object_list);
	mtx_unlock(&kgi_pager_mtx);

	while ((m = TAILQ_FIRST(&vma->vm_object->un_pager.kgip.kgip_pglist)) != 0) {
		TAILQ_REMOVE(&vma->vm_object->un_pager.kgip.kgip_pglist, m, pageq);
		kgi_pager_putfake(m);
	}

	kgi_pager_deallocate_vma(vma);
}


/* KGI pager manages its own pages since the swapper is not supposed
 * to see them. The pages are listed in their object and also pointed
 * by the buffers that use them in the sublayers of KGI. This why
 * the new allocated page is passed to the nopage hook of KGI.
 */
static int
kgi_pager_getpages(vm_object_t object, vm_page_t *m, int count, int reqpage)
{
	vm_area_t vma;
	vm_offset_t offset;
	vm_paddr_t paddr;
	vm_page_t page;
	int i, ret;
	int prot;

	VM_OBJECT_LOCK_ASSERT(object, MA_OWNED);
	vma = object->handle;
	offset = m[reqpage]->pindex;
	VM_OBJECT_UNLOCK(object);
	prot = PROT_READ;	/* XXX should pass in? */

	/* Get a new page */
	page = kgi_pager_getfake();

	/* Get the corresponding physical address from KGI graphic resource */
	ret = vma->vm_ops->nopage(vma, page, (vm_offset_t) offset << PAGE_SHIFT, 
							  &paddr, prot);

	KASSERT(ret == 0,("kgi_pager_getpage: map function returns error"));

	/*
	 * Replace the passed in reqpage page with our own fake page and free up
	 * all of the original pages. The KGI pager allocates one page at a time
	 * which is certainly not optimal XXX.
	 */
	page->phys_addr = paddr;

	VM_OBJECT_LOCK(object);
	TAILQ_INSERT_TAIL(&object->un_pager.kgip.kgip_pglist, page, pageq);
	vm_page_lock_queues();
	for (i = 0; i < count; i++) {
		vm_page_free(m[i]);
	}
	vm_page_unlock_queues();
	vm_page_insert(page, object, offset);
	m[reqpage] = page;

	return (VM_PAGER_OK);
}

static void
kgi_pager_putpages(vm_object_t object, vm_page_t *m, int count,
		   boolean_t sync, int *rtvals)
{
	panic("kgi_pager_putpage called");
}

static boolean_t
kgi_pager_haspage(vm_object_t object, vm_pindex_t pindex,
		  int *before, int *after)
{
	if (before != NULL)
		*before = 0;
	if (after != NULL)
		*after = 0;
	return (TRUE);
}

struct pagerops kgipagerops = {
	.pgo_init 		= kgi_pager_init,
	.pgo_alloc 		= kgi_pager_alloc,
	.pgo_dealloc 	= kgi_pager_dealloc,
	.pgo_getpages 	= kgi_pager_getpages,
	.pgo_putpages 	= kgi_pager_putpages,
	.pgo_haspage 	= kgi_pager_haspage
};

static vm_area_t 
kgi_pager_vma_allocate(vm_ooffset_t foff, vm_ooffset_t size, vm_prot_t prot)
{
	vm_area_t vma;

	/* Allocate a new vm area */
	vma = (vm_area_t )uma_zalloc(vma_zone, M_WAITOK);
	bzero(vma, sizeof(struct vm_area_struct));

	/* Initialize the area */
	vma->vm_offset = foff;
	vma->vm_size = size;
	vma->vm_prot = prot;

	return (vma);
}

void
kgi_pager_remove(vm_area_t vma, vm_page_t m)
{
	
	VM_OBJECT_LOCK(vma->vm_object);
	vm_page_lock_queues();

	vm_page_busy(m);
	pmap_remove_all(m);
	vm_page_remove(m);

	vm_page_unlock_queues();

	TAILQ_REMOVE(&vma->vm_object->un_pager.kgip.kgip_pglist, m, pageq);
	VM_OBJECT_UNLOCK(vma->vm_object);

	/* Not optimal, should improve with a pool of pages of the pager. XXX */
	kgi_pager_putfake(m);
}

void
kgi_pager_remove_all(vm_area_t vma)
{
	vm_page_t m;
	int num = 0;

	while ((m = TAILQ_FIRST(&vma->vm_object->un_pager.kgip.kgip_pglist)) != 0) {
		kgi_pager_remove(vma, m);
		num ++;
	}

	printf("%d pages removed from vm area %p\n", num, vma);
}

static void
kgi_pager_deallocate_vma(vm_area_t vma)
{
	/* Free the vma */
	uma_zfree(vma_zone, vma);
}

static vm_page_t
kgi_pager_getfake(void)
{
	vm_page_t m;

	m = uma_zalloc(fakepg_zone, M_WAITOK);

	/* Compared to the device pager, KGI pages have a PV entry
	 * thus PG_FICTITIOUS is not used.
	 */
	m->flags = VPO_BUSY;
	m->valid = VM_PAGE_BITS_ALL;
	m->dirty = 0;
	m->busy = 0;
	m->queue = PQ_NONE;
	m->object = NULL;

	/* XXX HACK HACK HACK. This is MD code! */
	TAILQ_INIT(&m->md.pv_list);

	/* Force the page non pageable */
	m->wire_count = 1;
	m->hold_count = 0;

	return (m);
}

static void
kgi_pager_putfake(vm_page_t m)
{
	uma_zfree(fakepg_zone, m);
}
