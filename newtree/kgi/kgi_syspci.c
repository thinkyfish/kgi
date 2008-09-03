/* ---------------------------------------------------------------------------
**	KGI FreeBSD PCI system layer
** ---------------------------------------------------------------------------
**	Copyright (C)	2001	Nicholas Souchu - Alcôve
**	Copyright (C)	2003	Nicholas Souchu
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
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define	KGI_SYS_NEED_IO
#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_VM
#define KGI_SYS_NEED_USER
#include <dev/kgi/system.h>

#include <dev/kgi/kgi.h>
#include <dev/kgi/kgii.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#define	__KRN_BUF_SIZE	1024

LIST_HEAD(pcicfg_list, pcicfg_dev) pcicfg_head = LIST_HEAD_INITIALIZER(head);
LIST_HEAD(pcicfg_resource_list, pcicfg_resource);

struct pcicfg_resource {

	int type;

	int rid;
	struct resource *res;
	void *intrhand;
	union {
		io_region_t io;
		mem_region_t mem;
		irq_line_t irq;
	} region;

	LIST_ENTRY(pcicfg_resource) entries;
};

struct pcicfg_dev {

	pcicfg_vaddr_t pcidev;
	device_t dev;

#define PCICFG_CLAIMED	1
	int flags;

	struct pcicfg_resource_list resources;

	LIST_ENTRY(pcicfg_dev) entries;
};

static int initialized = 0;

/*
**	PCI configuration space
*/
struct pcicfg_dev *pcicfg_lookup_device(pcicfg_vaddr_t pcidev)
{
	struct pcicfg_dev *p;

	LIST_FOREACH(p, &pcicfg_head, entries) {
		if (p->pcidev == pcidev)
			return p;
	}

	return NULL;
}

/* lookup the first matching pcicfg device_t */
device_t pcicfg_get_device(pcicfg_vaddr_t pcidev)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(pcidev);

	if (p)
		return p->dev;

	return NULL;
}

pcicfg_vaddr_t pcicfg_dev2cfg(device_t dev)
{
	u_int32_t bus, slot, func;

	bus = pci_get_bus(dev);
	slot = pci_get_slot(dev);
	func = pci_get_function(dev);

	return PCICFG_VADDR(bus, slot, func); 
}

/* XXX not SMP safe */
int pcicfg_add_device(device_t dev)
{
	struct pcicfg_dev *p;
	pcicfg_vaddr_t pcidev;

	/* If not done, initialize the pcicfg system.
	 */
	if (!initialized) {
		LIST_INIT(&pcicfg_head);
		initialized = 1;
	}

	pcidev = pcicfg_dev2cfg(dev);

	/* Check if pci device already in the list.
	 */
	LIST_FOREACH(p, &pcicfg_head, entries) {
		if (p->pcidev == pcidev)
			return KGI_EOK;
	}

	p = (struct pcicfg_dev *)kgi_kmalloc(sizeof(struct pcicfg_dev));
	if (!p)
		return KGI_ENOMEM;

	memset(p, 0, sizeof(*p));

	/* Store the internal device for future reference.
	 */
	p->dev = dev;
	p->pcidev = pcidev;

	/* Initialize the list of resources for this device.
	 */
	LIST_INIT(&p->resources);

	/* Insert in the global list of pci configs.
	 */
	LIST_INSERT_HEAD(&pcicfg_head, p, entries);

	return KGI_EOK;
}

/* XXX not SMP safe */
int pcicfg_remove_device(device_t dev)
{
	struct pcicfg_dev *p;
	struct pcicfg_resource *r;

	if (!initialized)
		return KGI_EINVAL;

	/* Scan the list of pci configs.
	 */
	LIST_FOREACH(p, &pcicfg_head, entries) {
                if (p->dev == dev) {
			/* Delete the list of resources for this device.
			 */
			while (!LIST_EMPTY(&p->resources)) {
				r = LIST_FIRST(&p->resources);
				LIST_REMOVE(r, entries);

				/* XXX bus_xxx free the resources!! */
				kgi_kfree(r);
			}
			LIST_REMOVE(p, entries);
			kgi_kfree(p);

			return 0;
		}
        }

	return KGI_EINVAL;
}

/* claim the device so that no other driver will get it */
void pcicfg_claim_device(pcicfg_vaddr_t addr)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(addr);

	if (p)
		p->flags |= PCICFG_CLAIMED;
}

/* free the device so that other driver will get it */
void pcicfg_free_device(pcicfg_vaddr_t addr)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(addr);

	if (p)
		p->flags &= ~PCICFG_CLAIMED;
}

/*
 * Find the PCI device/subsystem in the list of registered devices.
 *
 * The PCI cfg space is actually the list of devices found by the pci bus. 
 *
 * A PCI device must have been previously scanned and found by the pci
 * bus (see board_probe()) for being found here.
 *
 * Actually, if a driver is going there, there's a lot of chance something
 * will be found.
 */
int pcicfg_find_device(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pcicfg_dev *p;
	const __kgi_u32_t *check;
	__kgi_u32_t signature;
	u_int16_t vendor, device;

	KRN_DEBUG(4, "scanning device in pcicfg space:");

	/* Search among registered boards */
	LIST_FOREACH(p, &pcicfg_head, entries) {

		/* If device already claimed, not a candidate */
		if (p->flags & PCICFG_CLAIMED)
			continue;

		device = pci_get_device(p->dev);
		vendor = pci_get_vendor(p->dev);

		KRN_DEBUG(4, "scanning device %x %x\n", vendor, device);

		signature = PCICFG_SIGNATURE(vendor, device);

		/* Scan all signatures for the given dev */
		check = signatures;
		while (*check && (*check != signature)) {

			check++;
		}

		/* Finally, the first dev after "after" is returned */
		if (*check && (*check == signature)) {

			*addr = pcicfg_dev2cfg(p->dev);

			KRN_DEBUG(4, "found device %.8x at %.8x", 
				  signature, *addr);
			return 0;
		}
	}

	*addr = PCICFG_NULL;
	return KGI_EINVAL;
}

int pcicfg_find_subsystem(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pcicfg_dev *p;
	const __kgi_u32_t *check;
	__kgi_u32_t signature;
	u_int16_t subvendor, subdevice;
	u_int16_t vendor, device;

	KRN_DEBUG(4, "scanning subsystem in pcicfg space:");

	/* Search among registered boards */
	LIST_FOREACH(p, &pcicfg_head, entries) {

		/* If device already claimed, not a candidate */
		if (p->flags & PCICFG_CLAIMED)
			continue;

		device = pci_get_device(p->dev);
		vendor = pci_get_vendor(p->dev);

		subdevice = pci_get_subdevice(p->dev);
		subvendor = pci_get_subvendor(p->dev);

		signature = PCICFG_SIGNATURE(subvendor, subdevice);

		KRN_DEBUG(4, "scanning device %x %x, subsystem %x %x\n", 
			vendor, device, subvendor, subdevice);

		check = signatures;
		while (*check && (*check != signature)) {

			check++;
		}

		if (*check && (*check == signature)) {

			*addr = pcicfg_dev2cfg(p->dev);

			KRN_DEBUG(4, "found device %.8x at %.8x", 
				  signature, *addr);
			return KGI_EOK;
		}
	}

	*addr = PCICFG_NULL;
	return KGI_ENODEV;
}

int pcicfg_find_class(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pcicfg_dev *p;
	const __kgi_u32_t *check;
	u_int16_t class;

	KRN_DEBUG(4, "scanning class in pcicfg space:");

	LIST_FOREACH(p, &pcicfg_head, entries) {

		/* If device already claimed, not a candidate */
		if (p->flags & PCICFG_CLAIMED)
			continue;

		class = pci_get_class(p->dev) << 8;

		KRN_DEBUG(4, "scanning device with class %.8x", class);

		check = signatures;
		while (*check && (*check != class)) {

			check++;
		}

		if (*check && (*check == class)) {

			*addr = pcicfg_dev2cfg(p->dev);

			KRN_DEBUG(4, "found device of class %.8x at %.8x", 
				  class, *addr);

			return KGI_EOK;
		}
	}

	*addr = PCICFG_NULL;
	return KGI_ENODEV;
}


#define VADDR_BASE (vaddr & ~0xFF)
#define VADDR_OFFSET (vaddr & 0xFF)

__kgi_u8_t  pcicfg_in8 (const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return -1;

	return pci_read_config(dev, VADDR_OFFSET, 1);
}

__kgi_u16_t pcicfg_in16(const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return -1;

	return pci_read_config(dev, VADDR_OFFSET, 2);
}

__kgi_u32_t pcicfg_in32(const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return -1;

	return pci_read_config(dev, VADDR_OFFSET, 4);
}

void pcicfg_out8 (const __kgi_u8_t val, const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return;

	pci_write_config(dev, VADDR_OFFSET, val, 1);

	return;
}

void pcicfg_out16(const __kgi_u16_t val, const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return;

	pci_write_config(dev, VADDR_OFFSET, val, 2);

	return;
}

void pcicfg_out32(const __kgi_u32_t val, const pcicfg_vaddr_t vaddr)
{
	device_t dev = pcicfg_get_device(VADDR_BASE);

	if (!dev)
		return;

	pci_write_config(dev, VADDR_OFFSET, val, 4);
}

/*
**	io I/O space
*/
__kgi_error_t io_check_region(io_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	struct resource *res = NULL;
	int rid = r->rid;

	/* Is the device registered? */
	if (!p)
		goto error;

	/* Try to allocate the resource */
	if (!(res = bus_alloc_resource(p->dev, SYS_RES_IOPORT, &rid,
				       r->base_io, r->base_io + (r->size -1),
				       r->size, RF_ACTIVE | RF_SHAREABLE))) {

		/* Try to set the resource before allocation */
		bus_set_resource(p->dev, SYS_RES_IOPORT, rid, r->base_io,
				 r->size);

		res = bus_alloc_resource(p->dev, SYS_RES_IOPORT, &rid,
					 r->base_io, r->base_io + (r->size -1),
					 r->size, RF_ACTIVE | RF_SHAREABLE);
	}

	/* Resource allocation succeeded, release then report ok */
	if (res) {
		bus_release_resource(p->dev, SYS_RES_IOPORT, rid, res);

		KRN_DEBUG(2, "io_check_region('%s', base 0x%x, size %i): ok",
			  r->name, r->base_io, r->size);

		return KGI_EOK;
	}

error:
	KRN_DEBUG(2, "io_check_region('%s', base 0x%x, size %i): "
		  "failed", r->name, r->base_io, r->size);
	return KGI_ENODEV;
}

io_vaddr_t io_claim_region(io_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	device_t dev = pcicfg_get_device(r->device);
	struct pcicfg_resource *pcicfg_res = NULL;
	struct resource *res = NULL;
	int rid = r->rid;

	pcicfg_res = (struct pcicfg_resource *)kgi_kmalloc(sizeof(struct
		pcicfg_resource));

	if (!p || !dev || !pcicfg_res)
		goto error;

	memset(pcicfg_res, 0, sizeof(*pcicfg_res));

	KRN_ASSERT(r->base_virt == 0); /* claim of claimed region? */

	if (!(res = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid,
				       r->base_io, r->base_io + (r->size -1),
				       r->size, RF_ACTIVE | RF_SHAREABLE)))
		goto error;

	r->base_virt = r->base_phys = r->base_bus = r->base_io;

	pcicfg_res->type = SYS_RES_IOPORT;
	pcicfg_res->rid = rid;
	pcicfg_res->res = res;
	pcicfg_res->region.io = *r;

	LIST_INSERT_HEAD(&p->resources, pcicfg_res, entries);

	KRN_DEBUG(2, "io_claim_region('%s', base_io %.4x, base_virt %.4x, "
		"base_phys %.4x, base_bus %.4x): success", r->name,
		r->base_io, r->base_virt, r->base_phys, r->base_bus);

	return r->base_virt;

error:
	KRN_DEBUG(2, "mem_claim_region('%s', base_io 0x%x, size %i): failed",
		  r->name, (kgi_u32_t)r->base_io, (kgi_u_t)r->size);
	if (pcicfg_res)
		kgi_kfree(pcicfg_res);
	return 0;
}

io_vaddr_t io_free_region(io_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	struct pcicfg_resource *pcicfg_res = NULL;
	int error;

	if (!p)
		return KGI_EINVAL;

	LIST_FOREACH(pcicfg_res, &p->resources, entries) {

		if ((pcicfg_res->type == SYS_RES_IOPORT) &&
		    (pcicfg_res->region.io.base_virt == r->base_virt) &&
		    (pcicfg_res->region.io.size == r->size)) {
			break;
		}
	}

	if (pcicfg_res) {
		KRN_DEBUG(2, "io_free_region('%s', base_io %.4x, base_virt %.4x, "
			  "base_phys %.4x, base_bus %.4x)", r->name,
			  r->base_io, r->base_virt, r->base_phys,
			  r->base_bus);
		
		error = bus_release_resource(p->dev, SYS_RES_IOPORT,
					     pcicfg_res->rid, pcicfg_res->res);
		
		LIST_REMOVE(pcicfg_res, entries);
		kgi_kfree(pcicfg_res);
		
		r->base_virt = 0;
		r->base_phys = r->base_bus = 0;
	}

	return 0;
}

io_vaddr_t io_alloc_region(io_region_t *r)
{
	/*	AFAIK nothing appropriate under Linux yet.
	*/
	return 0;
}

/*
**	memory I/O space
*/
__kgi_error_t mem_check_region(mem_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	struct resource *res = NULL;
	int rid = r->rid;

	/* Is the device registered? */
	if (!p)
		goto error;

	/* Try to allocate the resource */
	if (!(res = bus_alloc_resource(p->dev, SYS_RES_MEMORY, &rid,
				       r->base_io, r->base_io + (r->size -1),
				       r->size, RF_ACTIVE | RF_SHAREABLE))) {

		/* Try to set the resource before allocation */
		bus_set_resource(p->dev, SYS_RES_MEMORY, rid, r->base_io,
				 r->size);

		res = bus_alloc_resource(p->dev, SYS_RES_MEMORY, &rid,
					 r->base_io, r->base_io + (r->size -1),
					 r->size, RF_ACTIVE | RF_SHAREABLE);
	}

	/* Resource allocation succeeded, release then report ok */
	if (res) {
		bus_release_resource(p->dev, SYS_RES_MEMORY, rid, res);

		KRN_DEBUG(2, "mem_check_region('%s', base 0x%x, size %i): ok",
			  r->name, r->base_io, r->size);

		return KGI_EOK;
	}

error:
	KRN_DEBUG(2, "mem_check_region('%s', base 0x%x, size %i): "
		  "failed", r->name, r->base_io, r->size);
	return KGI_ENODEV;
}

mem_vaddr_t mem_claim_region(mem_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	device_t dev = pcicfg_get_device(r->device);
	struct pcicfg_resource *pcicfg_res = NULL;
	struct resource *res = NULL;
	int rid = r->rid;

	pcicfg_res = (struct pcicfg_resource *)kgi_kmalloc(sizeof(struct
		pcicfg_resource));

	if (!p || !dev || !pcicfg_res)
		goto error;

	memset(pcicfg_res, 0, sizeof(*pcicfg_res));

	if (!(res = bus_alloc_resource(dev, SYS_RES_MEMORY, &rid,
				       r->base_io, r->base_io + (r->size -1),
				       r->size, RF_ACTIVE | RF_SHAREABLE)))
		goto error;

	r->base_phys = (mem_paddr_t) r->base_io;
	r->base_bus  = (mem_baddr_t) r->base_io;
	r->base_virt = (mem_vaddr_t) rman_get_virtual(res);

	pcicfg_res->type = SYS_RES_MEMORY;
	pcicfg_res->rid = rid;
	pcicfg_res->res = res;
	pcicfg_res->region.mem = *r;

	LIST_INSERT_HEAD(&p->resources, pcicfg_res, entries);

	KRN_DEBUG(2, "mem_claim_region('%s', base_io %p, base_virt %p, "
		"base_phys %p, base_bus %p): success", r->name, 
		(void *) r->base_io, (void *) r->base_virt, 
		(void *) r->base_phys, (void *) r->base_bus);

	return r->base_virt;

error:
	KRN_DEBUG(2, "mem_claim_region('%s', base_io 0x%x, size %i): failed",
		  r->name, r->base_io, r->size);
	if (pcicfg_res)
		kgi_kfree(pcicfg_res);
	return (mem_vaddr_t)NULL;
}

mem_vaddr_t mem_free_region(mem_region_t *r)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(r->device);
	struct pcicfg_resource *pcicfg_res = NULL;
	int error;

	if (!p)
		return (mem_vaddr_t)NULL;

	LIST_FOREACH(pcicfg_res, &p->resources, entries) {

		if ((pcicfg_res->type == SYS_RES_MEMORY) &&
		    (pcicfg_res->region.mem.base_virt == r->base_virt) &&
		    (pcicfg_res->region.mem.size == r->size)) {
			break;
		}
	}

	if (pcicfg_res) {
		KRN_DEBUG(2, "mem_free_region %s, base_io %p, base_virt %p, "
			  "base_phys %p, base_bus %p", r->name,
			  (void *) r->base_io, (void *) r->base_virt,
			  (void *) r->base_phys, (void *) r->base_bus);
	
		error = bus_release_resource(p->dev, SYS_RES_MEMORY,
					     pcicfg_res->rid, pcicfg_res->res);
		
		LIST_REMOVE(pcicfg_res, entries);
		kgi_kfree(pcicfg_res);
		
		r->base_bus  = (mem_baddr_t)NULL;
		r->base_phys = (mem_paddr_t)NULL;
		r->base_virt = (mem_vaddr_t)NULL;
	}
		
	return (mem_vaddr_t)NULL;
}

mem_vaddr_t mem_alloc_region(mem_region_t *r)
{
	/*	AFAIK there is no resource management for memory regions
	**	in Linux yet. We just report it failed.
	*/
	return (mem_vaddr_t)NULL;
}

/*
**	irq handling
*/
static void irq_handler(void *priv)
{
	register irq_line_t *irq = (irq_line_t *) priv;

	if (irq->High) {

		irq->High(irq->meta, irq->meta_io, NULL);
	}

	return;
}

__kgi_error_t irq_claim_line(irq_line_t *irq)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(irq->device);
	device_t dev = pcicfg_get_device(irq->device);
	struct pcicfg_resource *pcicfg_res = NULL;
	struct resource *res = NULL;
	int rid = irq->line;
	unsigned int flags = RF_ACTIVE;
	int error;

	pcicfg_res = (struct pcicfg_resource *)kgi_kmalloc(sizeof(struct
		pcicfg_resource));

	if (!p || !dev || !pcicfg_res)
		goto error;

	memset(pcicfg_res, 0, sizeof(*pcicfg_res));

	if (irq->flags & IF_SHARED_IRQ) {

		flags |= RF_SHAREABLE;
	}
	if (!(res = bus_alloc_resource(dev, SYS_RES_IRQ, &rid,
				       0, ~0, 1, flags)))
		goto error;

	if ((error = bus_setup_intr(p->dev, res, INTR_TYPE_MISC,
			NULL, irq_handler, p, &pcicfg_res->intrhand)))
		goto error;

	pcicfg_res->type = SYS_RES_IRQ;
	pcicfg_res->rid = rid;
	pcicfg_res->res = res;
	pcicfg_res->region.irq = *irq;

	LIST_INSERT_HEAD(&p->resources, pcicfg_res, entries);

	KRN_DEBUG(2, "irq_claim_line('%s', line %i): success", irq->name,
		irq->line);

	return KGI_EOK;

error:
	KRN_DEBUG(2, "irq_claim_line('%s', line %i): failed", irq->name,
		irq->line);
	if (res)
		bus_release_resource(p->dev, SYS_RES_IRQ, 0, res);
	if (pcicfg_res)
		kgi_kfree(pcicfg_res);
	return KGI_EINVAL;
}

void irq_free_line(irq_line_t *irq)
{
	struct pcicfg_dev *p = pcicfg_lookup_device(irq->device);
	struct pcicfg_resource *pcicfg_res = NULL;

	LIST_FOREACH(pcicfg_res, &p->resources, entries) {

		if ((pcicfg_res->type == SYS_RES_IRQ) &&
		    (pcicfg_res->region.irq.line == irq->line)) {
			break;
		}
	}

	if (pcicfg_res) {

		KRN_DEBUG(2, "irq_free_line('%s', line %i)", irq->name, irq->line);

		bus_teardown_intr(p->dev, pcicfg_res->res, pcicfg_res->intrhand);
		bus_release_resource(p->dev, SYS_RES_IRQ, 0, pcicfg_res->res);
		
		LIST_REMOVE(pcicfg_res, entries);
		kgi_kfree(pcicfg_res);
	}

	return;
}
