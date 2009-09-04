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
 * FreeBSD KGI pager.
 */

#ifndef	_KGI_PAGER_
#define	_KGI_PAGER_

#include <sys/queue.h>

/* Head of the list of vma areas managed by the pager. */
TAILQ_HEAD(vmalst, vm_area_struct);

typedef struct vm_area_struct * vm_area_t;

struct vm_operations_struct {
	void (*open)(vm_area_t vma);
	void (*close)(vm_area_t vma);
	int (*nopage)(vm_area_t vma, vm_page_t m, vm_offset_t ooffset,
		      vm_paddr_t *paddr, int prot);
	void(*unmap)(vm_area_t vma);
};

/* The vm_area_struct is used to keep somehow compatibility with
 * Linux interface. This is also to keep graph_mapping_t as independent
 * of OS as possible.
 *
 * A vma is associated to each KGI pager VM object. The VMA struct somehow
 * duplicates the role of the object but adds the fields which
 * we don't want to add to the object structure.
 */
struct vm_area_struct {
	TAILQ_ENTRY(vm_area_struct) pager_vma_list;

	vm_ooffset_t vm_offset;		/* offset inside the object given at
					 * allocation */
	vm_ooffset_t vm_size;		/* size of the mapped area */
	vm_prot_t vm_prot;		/* area protection */

	vm_object_t vm_object;		/* the object of this area */

	struct vm_operations_struct *vm_ops; /* hook operations of the underlying
					      * mapping type (mmio, accel...) */

	void *vm_private_data;		/* actually the graph map, but void *
					 * to avoid type dependency */

	int vm_unit;			/* minor of the graphic dev */
	int vm_type;			/* type of the corresponding kgi resource */
};

/* 
 * Functions provided by the KGI device (accel or mmio for example).
 */
extern int graph_mmap(struct cdev *dev, vm_area_t vma);
extern void graph_munmap(vm_area_t vma);

/* 
 * Lowlevel routines provided to KGI devices to perform VM
 * operations.
 */
extern void kgi_pager_remove(vm_area_t vma, vm_page_t m);
extern void kgi_pager_remove_all(vm_area_t vma);

#endif /* _KGI_PAGER_ */