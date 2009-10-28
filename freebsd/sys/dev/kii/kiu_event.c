/*-
* Copyright (c) 2000 Steffen Seeger
* Copyright (c) 2001 Nicholas Souchu - Alcôve
* Copyright (c) 2002-2003 Nicholas Souchu
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
 * /dev/event special device file driver 
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/poll.h>
#include <sys/uio.h>

#define	KGI_SYS_NEED_IO
#define KII_NEED_MODIFIER_KEYSYMS
#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kii/event.h>

static struct cdevsw kiu_cdevsw;

#define	EVENT_MAX_IO_BUF_SIZE	PAGE_SIZE
#define	EVENT_MAX_NR_DEVICES	64
#define	EVENT_MAX_NR_IMAGES	64

struct wq_entry {
	struct thread *td;
	TAILQ_ENTRY(wq_entry) others;
};

TAILQ_HEAD(wait_queue, wq_entry);

/* per-device static global data. */
struct event_entry {
	unsigned int	cnt;
	event_device_t	*ptr;
	pid_t		pid;
	gid_t		gid;
};

/* per-device global data. */
struct event_entry event_dev[EVENT_MAX_NR_DEVICES];

/* Maximum number of opened event files. */
#define EVENT_MAX_NR_FILES	16

/* 
 * Array of event files. One per process opening a device.
 * Each event file entry maintains per-process data along its
 * interaction with the event device.
 */
static event_file_t *event_files[EVENT_MAX_NR_FILES];

static kii_s_t event_device_init(kii_s_t device_id, struct thread *td);
static void event_device_done(kii_u_t device_id, kii_u_t previous);

static kii_u_t 
event_device_unit(struct cdev *dev, struct thread *td)
{
	kii_u_t unit = -1;

	if (dev2unit(dev))
		unit = dev2unit(dev) - 1;
	else
		if (td)
			unit = dev2unit(td->td_proc->p_pgrp->pg_session->
				s_ttyp->t_dev);
		else
			unit = 0;
	
	return (unit);
}

/*
 * Command resource.
 */
static int 
event_command(event_file_t *file, unsigned int cmd, void *data,
		struct thread *td)
{
	
	switch (cmd) {
	case KIIC_MAPPER_IDENTIFY: {
		/*
		 * For now only force client identification and
		 * identify myself (no compatibility checks).
		 */
		kiic_mapper_identify_result_t *out = data;

		if (file->flags & EVENT_FF_CLIENT_IDENTIFIED) 
			return (KII_EPROTO);

		file->flags |= EVENT_FF_CLIENT_IDENTIFIED;

		memset(out->mapper, 0, sizeof(out->mapper));
		strncpy(out->mapper, EVENT_NAME, sizeof(out->mapper));
		out->mapper_version.major = 0;
		out->mapper_version.minor = 9;
		out->mapper_version.patch = 0;
		out->mapper_version.extra = 0;
		/* XXX #warning tell client if it is session leader or not. */

		return (KII_EOK); 
		}
	}
	/* All commands below require identification. */
	if ((file->flags & EVENT_FF_CLIENT_IDENTIFIED) == 0) {
		KRN_DEBUG(1, "cmd = %.8x, but client has not yet identified",
				cmd);
		return (KII_EPROTO);
	}
#define	MUST_BE_SESSION_LEADER					\
	if ((file->flags & EVENT_FF_SESSION_LEADER) == 0) {	\
		KRN_DEBUG(1, "client is not session leader");	\
		return KII_EPROTO;				\
	}
	switch (cmd) {
	case KIIC_MAPPER_MAP_DEVICE: {
		kii_device_t *prev;

		MUST_BE_SESSION_LEADER

		prev = kii_current_focus(file->device->kii.focus_id);
		if (prev) {
			KRN_DEBUG(1, "unmapping previous device");
			switch (kii_unmap_device(prev->id)) {
			case KII_EOK:
				file->previous = prev->id;
				break;
			default:
				KRN_DEBUG(1, "can't unmap current");
				return (KII_EBUSY);
			}
		KRN_DEBUG(1, "mapping new device");
		kii_map_device(file->device->kii.id);
		return (KII_EOK);
		}
	}
	case KIIC_MAPPER_UNMAP_DEVICE:
		MUST_BE_SESSION_LEADER
		return (kii_unmap_device(file->device->kii.id));
	case KIIC_MAPPER_GET_KEYMAP_INFO: {
		kii_focus_t *focus = kiidev_focus(file->device->kii.id);
		kiic_mapper_get_keymap_info_result_t *out = data;

		memset(out, 0, sizeof(*out));

		if (focus == NULL) 
			return (KII_EINVAL);

		out->fn_buf_size  = focus->kmap.fn_buf_size;
		out->fn_str_size  = focus->kmap.fn_str_size;
		out->keymin       = focus->kmap.keymin;
		out->keymax       = focus->kmap.keymax;
		out->keymap_size  = focus->kmap.keymap_size;
		out->combine_size = focus->kmap.combine_size;

		return (KII_EOK);
	}
	case KIIC_MAPPER_GET_KEYMAP: {
		kiic_mapper_get_keymap_request_t local = 
			*(kiic_mapper_get_keymap_request_t *)data;
		kii_focus_t *focus = kiidev_focus(file->device->kii.id);
		kiic_mapper_get_keymap_request_t *in = &local;
		kiic_mapper_get_keymap_result_t *out = data;
		kii_u_t cnt;

		if ((focus == NULL) ||
			(focus->kmap.keymap_size <= in->keymap) ||
			(in->keymin > focus->kmap.keymax) ||
			(in->keymax < focus->kmap.keymin)){
			return (KII_EINVAL);
		}
		out->keymap = in->keymap;
		out->keymin = (in->keymin < focus->kmap.keymin)
			? focus->kmap.keymin : in->keymin;
		out->keymax = (in->keymax > focus->kmap.keymax)
			? focus->kmap.keymax : in->keymax;
		if (out->keymax < out->keymin) 
			out->keymax = out->keymin;

		cnt = out->keymax - out->keymin + 1;

		KRN_DEBUG(1, "put keymap %i, keys %i-%i, cnt %i",
			out->keymap, out->keymin, out->keymax, cnt);

		if (sizeof(out->map)/sizeof(out->map[0]) < cnt) {
			KRN_DEBUG(1, "keymap request > %i keys",
				sizeof(out->map)/sizeof(out->map[0]));
			return (KII_EINVAL);
		}

		if (focus->kmap.keymap[out->keymap]) {
			kii_unicode_t *map;

			KRN_DEBUG(1, "map %i @ %p", out->keymap,
				focus->kmap.keymap[out->keymap]);
			map = focus->kmap.keymap[out->keymap] +
				(out->keymin - focus->kmap.keymin);
			memcpy(out->map, map,cnt*sizeof(kii_unicode_t));
		} else {
			KRN_DEBUG(1, "map does not exist, %i VOIDs",cnt);
			while (cnt--) 
				out->map[cnt] = K_VOID;
		}
		return (KII_EOK);
	}
	default:
		KRN_DEBUG(1, "command %.4x not (yet) implemented", cmd);
		return (KII_ENXIO);
	}
}

/* ioctl() services. */
static int 
event_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flags,    
	  struct thread *td)
{
	kii_u_t unit;
	event_file_t *file;
	int io_result = KGI_EOK;

	/* Unit 0 is used to get a free /dev/event unit. */
	if (dev2unit(dev) == 0) {
		kiic_mapper_get_unit_request_t local = 
			*(kiic_mapper_get_unit_request_t *)data;
		kiic_mapper_get_unit_result_t *out = (void *)data;

		if (cmd != KIIC_MAPPER_GET_UNIT)
			return (KII_EINVAL);

		/* First try the suggested unit. */
		unit = local.unit;
		if (!KII_VALID_DEVICE_ID(unit) || event_files[unit]) {
			/* Then try the unit corresponding to the thread VT. */
			unit = event_device_unit(dev, td);
			
			/* If the VT unit is not free lookup one. */
			if (event_files[unit]) {
				for (unit = 0; unit < EVENT_MAX_NR_FILES; 
					unit++)
					if (event_files[unit] == 0)
						break;
				if (unit >= EVENT_MAX_NR_FILES)
					return (KII_ENODEV);
			}
		}
			
		file = kgi_kmalloc(sizeof(*file));
		if (file == NULL) {
			KRN_DEBUG(1, "failed to allocate event_file");
			return (KII_ENOMEM);
		}
		memset(file, 0, sizeof(*file));
		file->previous = KII_INVALID_DEVICE;

		event_files[unit] = file;
		unit ++;
		KRN_DEBUG(1, "allocating /dev/event%i", unit);

		/* Create the dev entry. */
		make_dev(&kiu_cdevsw, unit, UID_ROOT, GID_WHEEL,
			 0600, EVENT_NAME "%d", unit);
	
		out->unit = unit;
		return (KGI_EOK);
	}

	/* Unit should not have been opened if file is null. */
	unit = dev2unit(dev) - 1;
	file = event_files[unit];

	/* Attach the process to a specific graph device. */
	if (cmd == KIIC_MAPPER_ATTACH) {
		kiic_mapper_attach_request_t *in;

		if (file->device) {
			KRN_ERROR("ioctl() failed: device already attached");
			return (KII_EINVAL);
		}

		/* 
		 * If the application provides an invalid device_id
		 * we are supposed to take the default device provided
		 * by the thread tty minor.
		 */
		in = (kiic_mapper_attach_request_t *)data;

		if (!KII_VALID_DEVICE_ID(in->device_id))
			in->device_id = unit;

		if ((io_result = event_device_init(in->device_id, td))) {
			KRN_DEBUG(2, "failed to initialize event device %i",
				  in->device_id);
			return (io_result);
		}

		file->device_id = in->device_id;
		file->device = event_dev[in->device_id].ptr;

		if (event_dev[in->device_id].pid == td->td_proc->p_pid) 
			file->flags |= EVENT_FF_SESSION_LEADER;

		file->next = event_dev[in->device_id].ptr->files;
		event_dev[in->device_id].ptr->files = file;

		KRN_DEBUG(2, "ioctl: attached event device %i", in->device_id);
		return (KGI_EOK);
	}

	if (file->device == 0) {
		KRN_ERROR("ioctl() failed: no device attached");
		return KII_ENXIO;
	}

	mtx_lock(&file->device->cmd_mutex);

	switch (cmd & KIIC_TYPE_MASK) {
	case KIIC_MAPPER_COMMAND:
		io_result = event_command(file, cmd, data, td);
		break;
	default:
		KRN_DEBUG(1, "command type %.4x not (yet) implemented",
			  (kii_u_t)(cmd & KIIC_TYPE_MASK));
		io_result = (KGI_EINVAL);
		goto unlock;
	}

unlock:
	mtx_unlock(&file->device->cmd_mutex);
	return (io_result);
}

/*
 * kii_device functions.
 */
static void 
event_map_kii(kii_device_t *dev)
{

	KRN_DEBUG(2, "event_device %i mapped", dev->id);
}

static kii_s_t 
event_unmap_kii(kii_device_t *dev)
{

	KRN_DEBUG(2, "event_device %i unmapped", dev->id);
	return (KII_EOK);
}

static struct wait_queue waitqueue = TAILQ_HEAD_INITIALIZER(waitqueue);

static void 
event_handle_event(kii_device_t *dev, kii_event_t *ev)
{
	event_device_t *device = dev->priv.priv_ptr;
	event_file_t *file;

	KRN_DEBUG(3, "type %i, size %i, time %i", 
		ev->any.type, ev->any.size, ev->any.time);

	mtx_lock(&device->cmd_mutex);

	for (file = device->files; file; file = file->next) {
		kii_u_t new_head = file->queue.head + ev->size;

		KRN_ASSERT(device->files->device_id == file->device_id);
		KRN_ASSERT(file->queue.tail < file->queue.size);
		KRN_ASSERT(file->queue.tail <= file->queue.head);

		KRN_DEBUG(3, "queuing for file %p, head %i, tail %i",
			file, file->queue.head, file->queue.tail);

		if (new_head < file->queue.size) {
			KRN_ASSERT(file->queue.head < file->queue.size);
			memcpy(file->queue.buffer + file->queue.head,
				ev, ev->size);
			file->queue.head = new_head;
			continue;
		}

		if (file->queue.tail <= (new_head - file->queue.size)) {
			KRN_DEBUG(1, "queue overrun (file %p), event dropped",
				file);
			file->flags |= EVENT_FF_QUEUE_OVERRUN;
			continue;
		}

		if (file->queue.size < file->queue.head) {
			KRN_ASSERT(file->queue.head < 2 * file->queue.size);

			memcpy(file->queue.buffer + file->queue.head - 
				file->queue.size, ev, ev->size);
			file->queue.head = new_head;
			continue;
		}

		memcpy(file->queue.buffer + file->queue.head, ev,
			file->queue.size - file->queue.head);
		memcpy(file->queue.buffer, ev,
			new_head - file->queue.size);
		file->queue.head = new_head;
	}

	KRN_ASSERT(device->files);

	mtx_unlock(&device->cmd_mutex);

	wakeup(&waitqueue);
}

/*
 * event_device functions.
 */
static int 
event_read(struct cdev *dev, struct uio *uio, int ioflag)
{
	event_file_t *file;
	int error, count;
	struct wq_entry wait;

	/* 
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 */
	if (dev2unit(dev) == 0)
		return (EINVAL);

	file = event_files[dev2unit(dev) - 1];

	if (file->device == 0)
		return (EINVAL);

	mtx_lock(&file->device->cmd_mutex);

	error = 0;
	if (file->queue.head == file->queue.tail) {
		TAILQ_INSERT_TAIL(&waitqueue, &wait, others);

		while (error == 0 && (file->queue.head == file->queue.tail)) {
			error = msleep((caddr_t)&waitqueue, 
				  &file->device->cmd_mutex, PWAIT | PCATCH,
				  "kiuvent", 0);
		}

		TAILQ_REMOVE(&waitqueue, &wait, others);

		if (error)
			goto unlock;
	}

	count = (uio->uio_resid > file->queue.head - file->queue.tail) ?
		file->queue.head - file->queue.tail : uio->uio_resid;

	KRN_ASSERT(count > 0);

	/* XXX Can't lock while using uiomove. */
	mtx_unlock(&file->device->cmd_mutex);

	while (count) {
		if (uio->uio_resid > 0) {
			if ((error = uiomove(&file->queue.buffer[file->
					queue.tail], 1, uio)))
				return error;
		}
		count--;
		file->queue.tail++;
		if (file->queue.tail < file->queue.size) 
			continue;

		KRN_ASSERT(file->queue.tail <= file->queue.head);
		file->queue.tail -= file->queue.size;
		file->queue.head -= file->queue.size;
	}

	return (0);

 unlock:
	mtx_unlock(&file->device->cmd_mutex);
	return (error);
}

static int 
event_poll(struct cdev *dev, int poll, struct thread *td)
{
	event_file_t *file;

	/* 
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 */
	if (dev2unit(dev) == 0)
		return (EINVAL);

	file = event_files[dev2unit(dev) - 1];

	return ((file->queue.tail == file->queue.head) 
		? 0 
		: (POLLIN | POLLRDNORM));
}

static kii_s_t 
event_device_init(kii_s_t device_id, struct thread *td)
{
	int err;
	event_device_t *device;

	KRN_ASSERT(device_id >= 0);

	if (event_dev[device_id].ptr) {
		KRN_DEBUG(1, "event_device %i already initialized", device_id);
		event_dev[device_id].cnt++;
		return (KII_EOK);
	}
	if (event_dev[device_id].cnt) {
		KRN_DEBUG(1, "event_device %i has pending (mmap) references",
				device_id);
		return (KII_EBUSY);
	}
	KRN_ASSERT(event_dev[device_id].pid == 0);
	KRN_ASSERT(event_dev[device_id].gid == 0);

	if ((device = kgi_kmalloc(sizeof(*device))) == NULL) {
		KRN_DEBUG(1, "failed to allocate event_device %i", device_id);
		return (KII_ENOMEM);
	}
	memset(device, 0, sizeof(*device));

	device->kii.id		= KII_INVALID_DEVICE;
	device->kii.focus_id	= KII_INVALID_FOCUS;
	device->kii.MapDevice	= event_map_kii;
	device->kii.UnmapDevice	= event_unmap_kii;
	device->kii.HandleEvent	= event_handle_event;
	device->kii.event_mask	= KII_EM_KEY | KII_EM_POINTER;
	device->kii.priv.priv_ptr = device;

	if ((err = kii_register_device(&device->kii, device_id)) != KII_EOK) {
		KRN_DEBUG(1, "failed to register kii_device (index %i)",
				device_id);
		kgi_kfree(device);
		return (err);
	}
	KRN_DEBUG(2, "registered kii_device %i on focus %i", 
		device->kii.id, device->kii.focus_id);

	mtx_init(&device->cmd_mutex, "kiu mutex lock", NULL, MTX_DEF);

	event_dev[device_id].pid = td->td_proc->p_pid;
	event_dev[device_id].gid = td->td_proc->p_pgrp->pg_id;
	event_dev[device_id].ptr = device;
	event_dev[device_id].cnt++;

	KRN_DEBUG(1, "event_device %i initialized", device_id);

	return (KII_EOK);
}

static void 
event_device_done(kii_u_t device_id, kii_u_t previous)
{
	event_device_t *device = event_dev[device_id].ptr;

	KRN_ASSERT(device_id >= 0);

	if (--event_dev[device_id].cnt) {
		KRN_DEBUG(1, "event %i closed", device_id);
		return;
	}

	if (KII_VALID_DEVICE_ID(device->kii.id)) {
		KRN_DEBUG(1, "device still registered (id %i)",
			  device->kii.id);

		if (device->kii.flags & KII_DF_FOCUSED) {
			kii_unmap_device(device->kii.id);
			if (KII_VALID_DEVICE_ID(previous))
			    kii_map_device(previous);
		}
		kii_unregister_device(&device->kii);
	}

	mtx_destroy(&device->cmd_mutex);

	kgi_kfree(device);
	event_dev[device_id].ptr = NULL;

	KRN_ASSERT(event_dev[device_id].cnt == 0);
	KRN_ASSERT(event_dev[device_id].pid == 0);
	KRN_ASSERT(event_dev[device_id].gid == 0);

	KRN_DEBUG(1, "event %i finally closed", device_id);
}

static int 
event_open(struct cdev *dev, int flag, int mode, struct thread *td)
{
	event_file_t *file;
	void *queue_buffer;

	KRN_DEBUG(1, "open() /dev/event%i", dev2unit(dev));

	/* 
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 * See event_ioctl().
	 */
	if (dev2unit(dev) == 0)
		return (0);

	if ((file = event_files[dev2unit(dev) - 1]) == NULL) {
		KRN_DEBUG(1, "open: unit not allocated");
		return (ENODEV);
	}
		
	/* 
	 * Event files are statically allocated.
	 * A event file can be opened only once.
	 * Device is not initialized here but at attachement
	 * (see graph_command()).
	 */
	if (file->refcnt) {
		KRN_DEBUG(1, "open: failed, already opened");
		return (EBUSY);
	}

	queue_buffer = kgi_kmalloc(EVENT_QUEUE_SIZE);
	if (queue_buffer == NULL) {
		KRN_DEBUG(1, "failed to allocate queue_buffer");
		return (ENOMEM);
	}

	file->queue.buffer = queue_buffer;
	file->queue.size = EVENT_QUEUE_SIZE;
	file->refcnt++;

	return (0);
}

static int 
event_release(struct cdev *dev, int flags, int mode, struct thread *td)
{
	kii_u_t unit;
	event_file_t *file;

	/* 
	 * Avoid further operations on unit 0,
	 * It's just there to allocate a free unit number.
	 */
	if (dev2unit(dev) == 0)
		return (0);

	unit = dev2unit(dev) - 1;
	if ((file = event_files[unit]) == NULL) {
		KRN_DEBUG(1, "release: unit not allocated");
		return (ENODEV);
	}

	KRN_DEBUG(1, "closing /dev/event%i (refcnt %li)", unit, file->refcnt);

	/* 
	 * Only remove the file from the device list of files
	 * if a device was registered.
	 */
	if (file->device) {
		if (event_dev[file->device_id].ptr->files == file) {
			event_dev[file->device_id].ptr->files = file->next;
		} else {
			event_file_t *prev = 
					event_dev[file->device_id].ptr->files;
			while (prev->next != file) {
				prev = prev->next;
			}
			KRN_ASSERT(prev->next == file);
			
			prev->next = file->next;
		}
		file->next = NULL;

		if (td->td_proc->p_pid == event_dev[file->device_id].pid) {
			KRN_DEBUG(1, "session leader (pid %i) closed "
				  "event_device %i",
				  event_dev[file->device_id].pid, 
				  file->device_id);
			event_dev[file->device_id].pid = 0;
			event_dev[file->device_id].gid = 0;
		}

		event_device_done(file->device_id, file->previous);
	}

	kgi_kfree(file->queue.buffer);
	file->queue.buffer = NULL;
	file->device = NULL;

	file->refcnt--;
	KRN_ASSERT(file->refcnt == 0);

	kgi_kfree(file);
	event_files[unit] = NULL;

	destroy_dev(dev);

	return (0);
}

static int 
dev_event_init(void)
{
	memset(&event_dev, 0, sizeof(event_dev));
	memset(event_files, 0, sizeof(event_files));

	TAILQ_INIT(&waitqueue);
	
	/* Unit 0 is named /dev/event */
	make_dev(&kiu_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, EVENT_NAME);

	/* Other units are created later through /dev/event. */
	return (0);
}

static struct cdevsw kiu_cdevsw = {
	.d_open =	event_open,
	.d_close =	event_release,
	.d_read =	event_read,
	.d_ioctl =	event_ioctl,
	.d_poll =	event_poll,
	.d_name =	EVENT_NAME,
	.d_flags =	0,
	.d_version =	D_VERSION
};

static int
kiu_modevent(module_t mod, int type, void *unused)
{
	int error = ENXIO;

	switch (type) {
	case MOD_LOAD:
		error = dev_event_init();
		break;
	case MOD_UNLOAD:
		/* 
		 * XXX 
		 * dev_event_done(); Destroy devs!
		 */
	default:
		break;
	}

	return (error);
}

static moduledata_t kiu_mod = {
	"kiu",
	kiu_modevent,
	0
};

DECLARE_MODULE(kiu, kiu_mod, SI_SUB_DRIVERS, SI_ORDER_FIRST);
MODULE_VERSION(kiu, 1);
