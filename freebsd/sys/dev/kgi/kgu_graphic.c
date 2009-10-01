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
#define KGI_SYS_NEED_PROC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>
#include <dev/kgi/mmio.h>
#include <dev/kgi/accel.h>

#include <sys/conf.h>
#include <sys/tty.h>

static struct cdevsw kgu_cdevsw;

/*
 * per-device static global data.
 */
static struct {
	unsigned int	cnt;
	graph_device_t	*ptr;
	pid_t		pid;
	gid_t		gid;
} graph_dev[GRAPH_MAX_NR_DEVICES];

/*
 * Array of graph files. One per process opening a device.
 * Each graph file entry maintains per-process data along its
 * interaction with the graphic device.
 */
graph_file_t *graph_files[GRAPH_MAX_NR_FILES];

static kgi_u_t 
graph_device_unit(struct cdev *dev, struct thread *td)
{
	kgi_u_t device_id = KGI_INVALID_DEVICE;

	if (dev2unit(dev))
		device_id = dev2unit(dev) - 1;
	else
		if (td)
			device_id = dev2unit(td->td_proc->p_pgrp->pg_session->s_ttyp->t_dev);
		else
			device_id = 0;

	return (device_id);
}

/*
 * Note that the device is not registered here but later when
 * it is attached.
 */
static kgi_s_t 
graph_device_init(kgi_s_t device_id, struct thread *td)
{
	graph_device_t *device;

	KRN_ASSERT(device_id >= 0);
	kgi_mutex_assert(&kgi_lock, KGI_MUTEX_OWNED);

	if (graph_dev[device_id].ptr) {
		KRN_DEBUG(1, "graph_device %i already initialized", device_id);
		graph_dev[device_id].cnt++;
		return (KGI_EOK);
	}
	if (graph_dev[device_id].cnt) {
		KRN_DEBUG(1, "graph_device %i has pending (mmap) references",
			device_id);
		return (KGI_EBUSY);
	}
	KRN_ASSERT(graph_dev[device_id].pid == 0);
	KRN_ASSERT(graph_dev[device_id].gid == 0);

	if (NULL == (device = kgi_kmalloc(sizeof(*device)))) {
		KRN_DEBUG(1, "failed to allocate graph_device %i", device_id);
		return (KGI_ENOMEM);
	}
	memset(device, 0, sizeof(*device));	
	device->kgi.id = KGI_INVALID_DEVICE;
	device->kgi.dpy_id = KGI_INVALID_DISPLAY;

	kgi_mutex_init(&device->cmd_mtx, "kgi_graph mutex lock");

	graph_dev[device_id].pid = td->td_proc->p_pid;
	graph_dev[device_id].gid = td->td_proc->p_pgrp->pg_id;
	graph_dev[device_id].ptr = device;
	graph_dev[device_id].cnt++;

	KRN_DEBUG(1, "graph_device %i initialized", device_id);
	return (KGI_EOK);
}

static void 
graph_device_done(kgi_u_t device_id, kgi_u_t previous)
{
	graph_device_t *device = graph_dev[device_id].ptr;

	KRN_ASSERT(device_id >= 0);
	kgi_mutex_assert(&kgi_lock, KGI_MUTEX_OWNED);

	if (--graph_dev[device_id].cnt) {
		KRN_DEBUG(1, "graph_device %i closed", device_id);
		return;
	}

	if (KGI_VALID_DEVICE_ID(device->kgi.id)) {
		KRN_DEBUG(1, "device still registered (id %i)",	device->kgi.id);

		if (device->kgi.flags & KGI_DF_FOCUSED) {
			kgi_unmap_device(device->kgi.id);
			if (KGI_VALID_DEVICE_ID(previous))
				kgi_map_device(previous);
		}
		kgi_unregister_device(&device->kgi);
		kgi_kfree(device->kgi.mode->dev_mode);
		device->kgi.mode->dev_mode = NULL;
	}

	kgi_mutex_done(&device->cmd_mtx);

	kgi_kfree(device);
	graph_dev[device_id].ptr = NULL;

	KRN_ASSERT(graph_dev[device_id].cnt == 0);
	KRN_ASSERT(graph_dev[device_id].pid == 0);
	KRN_ASSERT(graph_dev[device_id].gid == 0);

	KRN_DEBUG(1, "graph_device %i finally closed", device_id);
}

/*
 * ioctl() services. This is the primary command interface. Other 
 * driver-specific interfaces may be provided using mmio-regions or 
 * by giving direct access to registers where this is safe. 
 * Other OSes might even have other means.
 */
static int 
graph_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flags,
	  struct thread *td)
{
	kgi_u_t unit;
	graph_file_t *file;
	int io_result = 0;

	/* Unit 0 is used to get a free /dev/event unit. */
	if (!dev2unit(dev)) {
		kgic_mapper_get_unit_request_t local = 
			*(kgic_mapper_get_unit_request_t *)data;
		kgic_mapper_get_unit_result_t *out = (void *)data;
	  
		if (cmd != KGIC_MAPPER_GET_UNIT)
			return (KGI_EINVAL);

		kgi_mutex_lock(&kgi_lock);

		/* First try the suggested unit. */
		unit = local.unit;
		if (!KGI_VALID_DEVICE_ID(unit) || graph_files[unit]) {
			/* Then try the unit corresponding to the thread VT. */
			unit = graph_device_unit(dev, td);
			
			/* If the VT unit is not free lookup one. */
			if (graph_files[unit]) {
				for (unit=0; unit<GRAPH_MAX_NR_FILES; unit++)
					if (!graph_files[unit])
						break;
				if (unit >= GRAPH_MAX_NR_FILES)
					return (KGI_ENODEV);
			}
		}
		
		file = kgi_kmalloc(sizeof(*file));
		if (NULL == file) {
			KRN_DEBUG(1, "failed to allocate event_file");
			return (KGI_ENOMEM);
		}
		memset(file, 0, sizeof(*file));
		file->previous = KGI_INVALID_DEVICE;

		graph_files[unit] = file;
		unit ++;
		KRN_DEBUG(1, "allocating /dev/graphic%i", unit);

		/* Create the dev entry. */
		make_dev(&kgu_cdevsw, unit, UID_ROOT, GID_WHEEL,
			 0600, GRAPH_NAME "%d", unit);
	
		out->unit = unit;

		kgi_mutex_unlock(&kgi_lock);
		return (0);
	}

	/* Unit should not have been opened if file is null. */
	unit = dev2unit(dev) - 1;
	file = graph_files[unit];

	/* attach the process to a specific graph device */
	if (cmd == KGIC_MAPPER_ATTACH) {
		kgic_mapper_attach_request_t *in = (void *)data;

		if (file->device) {
			KRN_ERROR("ioctl() failed: device already attached");
			return (KGI_EINVAL);
		}

		if (!KGI_VALID_DEVICE_ID(in->device_id))
			in->device_id = unit;

		kgi_mutex_lock(&kgi_lock);

		if ((io_result = graph_device_init(in->device_id, td))) 			
			return (io_result);

		file->device = graph_dev[in->device_id].ptr;
		file->device_id = in->device_id;
		if (graph_dev[in->device_id].pid == td->td_proc->p_pid) 
			file->flags |= GRAPH_FF_SESSION_LEADER;

		KRN_DEBUG(2, "ioctl: attached graphics device %i", in->device_id);

		kgi_mutex_unlock(&kgi_lock);
		return (KGI_EOK);
	}

	if (!file->device) {
		KRN_ERROR("ioctl() failed: no device attached");
		return (KGI_ENXIO);
	}

	kgi_mutex_lock(&kgi_lock);

	/* XXX Don't lock yet
	   kgi_mutex_lock(&file->device->cmd_mtx); */

	switch (cmd & KGIC_TYPE_MASK) {
	case KGIC_MAPPER_COMMAND:
		io_result = graph_command(file, cmd, data, td);
		break;
	case KGIC_RESOURCE_COMMAND:
		io_result = graph_resource_command(file, cmd, data);
		break;
	case KGIC_DISPLAY_COMMAND:
		if (KGI_VALID_DEVICE_ID(file->device->kgi.id))
			io_result = kgidev_display_command(&file->device->kgi, cmd, data);
		else
			io_result = KGI_EPROTO;
		break;
	default:
		KRN_DEBUG(1, "command type %.4x not (yet) implemented",
			(kgi_u_t)(cmd & KGIC_TYPE_MASK));
		io_result = KGI_EINVAL;
		goto unlock;
	}

unlock:
	/* XXX Don't lock yet
	   kgi_mutex_unlock(&file->device->cmd_mtx); */

	kgi_mutex_unlock(&kgi_lock);

	return (io_result);
}

void 
graph_device_map(kgi_device_t *dev)
{

	KRN_ASSERT(KGI_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(dev->id >= KGI_MAX_NR_CONSOLES);
	
	if (!graph_dev[dev->id - KGI_MAX_NR_CONSOLES].ptr) {	
		KRN_DEBUG(2, "Could not find device %i (graph %i)", \
				  dev->id, dev->id - KGI_MAX_NR_CONSOLES);
		return;
	}

	/*
	 * Wakeup any process blocked waiting for the device
	 * to be mapped.
	 */
	kgi_wakeup(dev);
}

/*
 * Unmap the graphic device. This removes any mapped page
 * in order to interrupt a process trying to access the
 * device while not mapped.
 * 
 * kgi_lock owned. 
 */
kgi_s_t 
graph_device_unmap(kgi_device_t *dev)
{
	graph_device_t *gdev = (graph_device_t *)dev;
	int i;

	kgi_mutex_assert(&kgi_lock, KGI_MUTEX_OWNED);

	KRN_ASSERT(KGI_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(dev->id >= KGI_MAX_NR_CONSOLES);
	
	if (!graph_dev[dev->id - KGI_MAX_NR_CONSOLES].ptr) {	
		KRN_DEBUG(2, "Could not find device %i (graph %i)",
			  dev->id, dev->id - KGI_MAX_NR_CONSOLES);
		return (KGI_EOK);
	}

	for (i=0; i < __KGI_MAX_NR_RESOURCES; i++)
	{
		graph_mapping_t *map;

		map = gdev->mappings[i];

		/* Lookup every mapping on the device and unmap them */
		if (map) {
			do {
				map = map->other;
				graph_unmap_map(map);
			} while (gdev->mappings[i] != map);
		}
	}

	return (KGI_EOK);
}	

/*
 * Open a graphic special file. We force all processes that made
 * references mmap() to invalidate these before we allow any reallocation.
 */
static int 
graph_open(struct cdev *dev, int flags, int mode, struct thread *td)
{
	graph_file_t *file;

	/*
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 */
	if (!dev2unit(dev))
		return (0);

	kgi_mutex_lock(&kgi_lock);

	if (!(file = graph_files[dev2unit(dev) - 1])) {
		KRN_DEBUG(1, "open: unit not allocated");
		return (KGI_ENODEV);
	}

	/*
	 * Graph files are statically allocated.
	 * A graph file can be opened only once.
	 * Device is not initialized here but at attachement
	 * (see graph_command()).
	 */
	if (file->refcnt) {
		KRN_DEBUG(1, "open: failed, already opened");
		return (KGI_EBUSY);
	}

	file->refcnt++;

	kgi_mutex_unlock(&kgi_lock);

	return (KGI_EOK);
}

static int 
graph_release(struct cdev *dev, int flags, int mode, struct thread *td)
{
	graph_file_t *file;

	/* 
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 */
	if (!dev2unit(dev))
		return (0);

	kgi_mutex_lock(&kgi_lock);

	if (!(file = graph_files[dev2unit(dev) - 1])) {
		KRN_DEBUG(1, "open: unit not allocated");
		return (KGI_ENODEV);
	}

	KRN_DEBUG(1, "closing graph device %i (refcnt %i)", 
		  file->device_id, graph_dev[file->device_id].cnt);

	/*
	 * Delete all mappings for this file. The vm_area_structs remain
	 * valid until the process explicitely unmaps them; if they are
	 * referenced but have no valid vm_private_data field we send SIGBUS.
	 */
	while (file->mappings) {
		graph_mapping_t *map = file->mappings->next;
		vm_area_t vma = map->vma;

		/* Close any activity on the resource */
		if (vma->vm_ops && vma->vm_ops->close)
			vma->vm_ops->close(vma);
		
		/* Remove the map from the file */
		graph_delete_mapping(map);

		/* Free the map */
		kgi_kfree(map);
	}

	file->device = NULL;

	file->refcnt--;
	KRN_ASSERT(file->refcnt == 0);

	if (td->td_proc->p_pid == graph_dev[file->device_id].pid) {
		KRN_DEBUG(1, "session leader (pid %i) closed graph_device %i",
			  graph_dev[file->device_id].pid, file->device_id);
		graph_dev[file->device_id].pid = 0;
		graph_dev[file->device_id].gid = 0;
	}

	graph_device_done(file->device_id, file->previous);

	kgi_kfree(file);
	graph_files[dev2unit(dev) - 1] = NULL;

	kgi_mutex_unlock(&kgi_lock);

	destroy_dev(dev);

	return (0);
}

/*
 * graph_mmap() does the neccessary stuff to add a mapping to a file
 * according to the mmaped resource type.
 */
int 
graph_mmap(struct cdev *dev, vm_area_t vma)
{
	kgi_u_t unit = dev2unit(dev) - 1;		/* XXX see graph_open() */
	graph_file_t *file = graph_files[unit];
	graph_mapping_t *map;
	int err;

	if (file->mmap_setup.resource == NULL) {
		KRN_ERROR("mmap not set up");
		return (KGI_ENXIO);
	}

	/*
	 * Remember the minor of the graphic dev. It will
	 * be used later to find the file from the vma.
	 */
	vma->vm_unit = unit;
	vma->vm_type = file->mmap_setup.resource->type;

	switch (vma->vm_type & KGI_RT_MASK) {
	case KGI_RT_MMIO:
		if ((err = graph_mmio_mmap(vma, &file->mmap_setup, &map))) {
			KRN_ERROR("mmio mapping failed");
			return (err);
		}
		KRN_DEBUG(1, "mmio mapping succeeded");
		break;
	case KGI_RT_ACCEL:
		if ((err = graph_accel_mmap(vma, &file->mmap_setup, &map))) {
			KRN_ERROR("accel mapping failed");
			return (err);
		}
		KRN_DEBUG(1, "accel mapping succeeded");
		break;
	default:
		KRN_ERROR("unkown mapping type %.8x", vma->vm_type);
		return (KGI_ENXIO);
	}

	/* Link OS dependent and OS independent data */
	map->vma = vma;
	vma->vm_private_data = map;

	/* Link to the resource mapped */
	map->resource = file->mmap_setup.resource;

	/* Link the map to the file entry */
	map->file = file;
	graph_add_mapping(file, (graph_mapping_t *) map);

	/* Call the resource callback */
	if (vma->vm_ops && vma->vm_ops->open)
		vma->vm_ops->open(vma);

	KRN_DEBUG(1, "mapped resource %s @ %.8x, size %u",
		file->mmap_setup.resource->name, 
		(kgi_u_t)vma->vm_offset, (kgi_u_t)vma->vm_size);

	return (KGI_EOK);
}

static struct cdevsw kgu_cdevsw = {
	.d_open =	graph_open,
	.d_close =	graph_release,
	.d_ioctl =	graph_ioctl,
	.d_name =	GRAPH_NAME,
	.d_flags =	D_KGI_PAGING,
	.d_version =	D_VERSION
};

static int 
dev_graphic_init(void)	
{

	memset(&graph_dev, 0, sizeof(graph_dev));
	memset(graph_files, 0, sizeof(graph_files));
  
	/* Unit 0 is named /dev/graphic */
	make_dev(&kgu_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, GRAPH_NAME);

  	return (KGI_EOK);
}
  
static int
kgu_modevent(module_t mod, int type, void *unused)
{
	int error = KGI_ENXIO;
  
	switch (type) {
	case MOD_LOAD:
		error = dev_graphic_init();
		break;
	case MOD_UNLOAD:
		/* XXX dev_graphic_done(); Destroy devs! */
	default:
		break;
	}

	return (error);
}

static moduledata_t kgu_mod = {
	"kgu",
	kgu_modevent,
	0
};

DECLARE_MODULE(kgu, kgu_mod, SI_SUB_DRIVERS, SI_ORDER_FIRST);
MODULE_VERSION(kgu, 1);
