/* ---------------------------------------------------------------------------
**	/dev/event special device file driver
** ---------------------------------------------------------------------------
**	Copyright (C)	2000		Steffen Seeger
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
**	$Log: event.c,v $
**	Revision 1.1  2000/09/21 09:14:34  seeger_s
**	- added first version of /dev/event mapper
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#undef	DEBUG
#define	DEBUG_LEVEL	2

#include <linux/version.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <asm/io.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
typedef struct wait_queue *wait_queue_head_t;
#define	init_waitqueue_head(head)	*((wait_queue_head_t *)(head)) = NULL
#define	set_current_state(a)		current->state = (a)
#endif

#include <linux/kgii.h>
#include <kii/event.h>

#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/wrapper.h>

#define	EVENT_MAX_IO_BUF_SIZE	PAGE_SIZE
#define	EVENT_MAX_NR_DEVICES	64
#define	EVENT_MAX_NR_IMAGES	64

/*	per-device static global data.
*/
static struct
{
	unsigned int	cnt;
	event_device_t	*ptr;
	pid_t		pid;
	gid_t		gid;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2)
	struct wait_queue	*proc_list;
#else
	struct wait_queue_head_t	*proc_list;
#endif
	struct fasync_struct *fasync;

} event_dev[EVENT_MAX_NR_DEVICES];

/* ----------------------------------------------------------------------------
**	helper functions
** ----------------------------------------------------------------------------
*/
static inline int event_device_id(kdev_t device)
{
	if (MAJOR(device) == EVENT_MAJOR) {

		if (MINOR(device)) {

			return MINOR(device) - 1;
		}

		if (current && current->tty && 
			(MAJOR(current->tty->device) == TTY_MAJOR)) {

			return MINOR(current->tty->device) - 1;
		}
	}

	KRN_DEBUG(1, "Don't know how to map %x to event minor.",
		device);
	return -1;
}


/*
**	command resource
*/
static inline int event_command(event_file_t *file, unsigned int cmd, 
	void *in_buffer, void **out_buffer, unsigned long *out_size)
{
	switch (cmd) {

	case KIIC_MAPPER_IDENTIFY:
		{	/*	For now only force client identification and
			**	identify myself (no compatibility checks).
			*/
			kiic_mapper_identify_result_t	*out = *out_buffer;
			KRN_ASSERT(*out_size >= sizeof(*out));
			*out_size = sizeof(*out);

			if (file->flags & EVENT_FF_CLIENT_IDENTIFIED) {

				return -EPROTO;
			}

			file->flags |= EVENT_FF_CLIENT_IDENTIFIED;

			memset(out->mapper, 0, sizeof(out->mapper));
			strncpy(out->mapper, EVENT_NAME, sizeof(out->mapper));
			out->mapper_version.major = 0;
			out->mapper_version.minor = 9;
			out->mapper_version.patch = 0;
			out->mapper_version.extra = 0;
#warning tell client if it is session leader or not.

			return KII_EOK;
		}
	}

	/*	all commands below require identification
	*/
	if (! (file->flags & EVENT_FF_CLIENT_IDENTIFIED)) {

		KRN_DEBUG(1, "cmd = %.8x, but client has not yet identified",
			cmd);
		return -EPROTO;
	}

#define	MUST_BE_SESSION_LEADER					\
	if (! (file->flags & EVENT_FF_SESSION_LEADER)) {	\
								\
		KRN_DEBUG(1, "client is not session leader");	\
		return -EPROTO;					\
	}

	switch (cmd) {

	case KIIC_MAPPER_MAP_DEVICE:
		{
			kii_device_t *prev;

			MUST_BE_SESSION_LEADER

			prev = kii_current_focus(file->device->kii.focus_id);
			if (prev) {

				KRN_DEBUG(1, "unmapping previous device");
				switch (kii_unmap_device(prev->id)) {

				case KII_EOK:
					break;
				default:
					KRN_DEBUG(1, "can't unmap current");
					return -EBUSY;
				}
			}
			KRN_DEBUG(1, "mapping new device");
			kii_map_device(file->device->kii.id);
			return KII_EOK;
		}

	case KIIC_MAPPER_UNMAP_DEVICE:
		{
			MUST_BE_SESSION_LEADER

			return kii_unmap_device(file->device->kii.id);
		}

	case KIIC_MAPPER_GET_KEYMAP_INFO:
		{
			kii_focus_t *focus = kiidev_focus(file->device->kii.id);
			kiic_mapper_get_keymap_info_result_t *out = *out_buffer;
			KRN_ASSERT(*out_size >= sizeof(*out));

			*out_size = sizeof(*out);
			memset(out, 0, sizeof(*out));

			if (NULL == focus) {

				return -EINVAL;
			}

			out->fn_buf_size  = focus->kmap.fn_buf_size;
			out->fn_str_size  = focus->kmap.fn_str_size;
			out->keymin       = focus->kmap.keymin;
			out->keymax       = focus->kmap.keymax;
			out->keymap_size  = focus->kmap.keymap_size;
			out->combine_size = focus->kmap.combine_size;

			return KII_EOK;
		}

	case KIIC_MAPPER_GET_KEYMAP:
		{
			kii_focus_t *focus = kiidev_focus(file->device->kii.id);
			kiic_mapper_get_keymap_request_t *in = in_buffer;
			kiic_mapper_get_keymap_result_t *out = *out_buffer;
			kii_unicode_t *map;
			kii_u_t cnt;

			if ((NULL == focus) ||
				(focus->kmap.keymap_size <= in->keymap) ||
				(in->keymin > focus->kmap.keymax) ||
				(in->keymax < focus->kmap.keymin)){

				return -EINVAL;
			}
			out->keymap = in->keymap;
			out->keymin = (in->keymin < focus->kmap.keymin)
				? focus->kmap.keymin : in->keymin;
			out->keymax = (in->keymax > focus->kmap.keymax)
				? focus->kmap.keymax : in->keymax;
			if (out->keymax < out->keymin) {

				out->keymax = out->keymin;
			}

			cnt = out->keymax - out->keymin + 1;
			*out_size = cnt * sizeof(kii_unicode_t) + 
				sizeof(kiic_mapper_get_keymap_request_t);

			KRN_DEBUG(1, "put keymap %i, keys %i-%i, cnt %i",
				out->keymap, out->keymin, out->keymax, cnt);

			if (sizeof(out->map)/sizeof(out->map[0]) < cnt) {

				KRN_DEBUG(1, "keymap request > %i keys",
					sizeof(out->map)/sizeof(out->map[0]));
				return -EINVAL;
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
				while (cnt--) {

					out->map[cnt] = K_VOID;
				}
			}

			return KII_EOK;
		}

	default:
		KRN_DEBUG(1, "command %.4x not (yet) implemented", cmd);
		return -ENXIO;
	}
}


/*	ioctl() services.
*/
static int event_ioctl(struct inode *inode, struct file *kfile,
	unsigned int cmd, unsigned long arg)
{
	event_file_t *file = (event_file_t *) kfile->private_data;
	int io_result	= KII_EOK;
	kii_size_t io_size	= EVENT_MAX_IO_BUF_SIZE;
	void *io_ibuf	= file->device->cmd_in_buffer;
	void *io_obuf	= file->device->cmd_out_buffer;

	down(&file->device->cmd_mutex);

	if (KIIC_READ(cmd)) {

		if (EVENT_MAX_IO_BUF_SIZE < KIIC_SIZE(cmd)) {

			KRN_DEBUG(1, "buffer too small for cmd %.8x", cmd);
			io_result = -ENOMEM;
			goto unlock;
		}
		io_result = verify_area(VERIFY_READ, (void *) arg,
			KIIC_SIZE(cmd));
		if (io_result != KII_EOK) {

			goto unlock;
	        }
		copy_from_user(io_ibuf, (void *) arg, KIIC_SIZE(cmd));

	} else {

		*((unsigned long *) io_ibuf) = arg;
	}

	switch (cmd & KIIC_TYPE_MASK) {

	case KIIC_MAPPER_COMMAND:
		io_result = event_command(file,
			cmd, io_ibuf, &io_obuf, &io_size);
		break;

	default:
		KRN_DEBUG(1, "command type %.4x not (yet) implemented",
			cmd & KIIC_TYPE_MASK);
		io_result = -EINVAL;
		goto unlock;
	}

	if (KIIC_WRITE(cmd) && (io_result == KII_EOK)) {

		io_result = verify_area(VERIFY_WRITE, (void *) arg, io_size);
		if (io_result != KII_EOK) {

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


/*
**	kii_device functions
*/
static void event_map_kii(kii_device_t *dev)
{
	KRN_DEBUG(2, "event_device %i mapped", dev->id);
}

static kii_s_t event_unmap_kii(kii_device_t *dev)
{
	KRN_DEBUG(2, "event_device %i unmapped", dev->id);
	return KII_EOK;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2)
		struct wait_queue wait = { current, NULL };
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
		DECLARE_WAIT_QUEUE(wait, current);
#else
		DECLARE_WAIT_QUEUE_HEAD(waitqueue);
#endif

static void event_handle_event(kii_device_t *dev, kii_event_t *ev)
{
	event_device_t *device = dev->priv.priv_ptr;
	event_file_t *file;

	KRN_DEBUG(3, "type %i, size %i, time %i", 
		ev->any.type, ev->any.size, ev->any.time);

	for (file = device->files; file; file = file->next) {

		kgi_u_t new_head = file->queue.head + ev->size;

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

		if (file->queue.size <= file->queue.head) {

			KRN_ASSERT(file->queue.head > file->queue.size);
			KRN_ASSERT(file->queue.head < 2*file->queue.size);

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	wake_up_interruptible((&(event_dev[device->files->device_id].proc_list)));
#else
	wake_up_interruptible(&waitqueue);
#endif
}

/*
**	event_device functions
*/
static ssize_t event_read(struct file *kfile, char *buffer,
	size_t count, loff_t *ppos)
{
	ssize_t retval;
	event_file_t *file = kfile->private_data;
	kii_event_t *event;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	DECLARE_WAITQUEUE(wait, current);
	if (file->queue.head == file->queue.tail) {
		add_wait_queue(&waitqueue, &wait);
#else
	if (file->queue.head == file->queue.tail) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2)
		struct wait_queue wait = { current, NULL };
#endif
		add_wait_queue(&(event_dev[file->device_id].proc_list), &wait);
#endif
		current->state = TASK_INTERRUPTIBLE;
		retval = 0;

		while (file->queue.head == file->queue.tail) {

			if ((kfile->f_flags & O_NONBLOCK) ||
				signal_pending(current)) {

				retval = -EAGAIN;
				break;
			}

			schedule();
		}

		current->state = TASK_RUNNING;
		
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
		remove_wait_queue(&event_dev[file->device_id].proc_list, &wait);
#else
		remove_wait_queue(&waitqueue, &wait);
#endif
		
		if (retval) {

			return retval;
		}
	}

	if ((file->queue.head - file->queue.tail) < count) {

		count = file->queue.head - file->queue.tail;
	}
	retval = count;
	KRN_ASSERT(count > 0);

	while (count--) {

		if (put_user(file->queue.buffer[file->queue.tail], buffer)) {

			return -EFAULT;
		}
		buffer++;
		file->queue.tail++;
		if (file->queue.tail < file->queue.size) {

			continue;
		}
		KRN_ASSERT(file->queue.tail <= file->queue.head);
		file->queue.tail -= file->queue.size;
		file->queue.head -= file->queue.size;
	}
	return retval;
}

static unsigned int event_poll(struct file *kfile, poll_table *wait)
{
	event_file_t *file = kfile->private_data;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	poll_wait(kfile, &(event_dev[file->device_id].proc_list), wait);
#else
	poll_wait(kfile, &waitqueue, wait);
#endif

	return (file->queue.tail == file->queue.head)
		? 0 : (POLLIN | POLLRDNORM);
}

static int event_fasync(int fd, struct file *kfile, int on)
{
	event_file_t *file = kfile->private_data;
	int retval;

	retval = fasync_helper(fd, kfile, on,
			&(event_dev[file->device_id].fasync));
	return (retval < 0)
		? retval : 0;
}

static int event_device_init(int device_id)
{
	int err;
	event_device_t *device;

	KRN_ASSERT(device_id >= 0);

	if (event_dev[device_id].ptr) {

		KRN_DEBUG(1, "event_device %i already initialized", device_id);
		event_dev[device_id].cnt++;
		return KII_EOK;
	}
	if (event_dev[device_id].cnt) {

		KRN_DEBUG(1, "event_device %i has pending (mmap) references",
			device_id);
		return -EBUSY;
	}
	KRN_ASSERT(event_dev[device_id].pid == 0);
	KRN_ASSERT(event_dev[device_id].gid == 0);

	if (NULL == (device = kmalloc(sizeof(*device), GFP_KERNEL))) {

		KRN_DEBUG(1, "failed to allocate event_device %i", device_id);
		return -ENOMEM;
	}
	memset(device, 0, sizeof(*device));

	device->kii.id		= KII_INVALID_DEVICE;
	device->kii.focus_id	= KII_INVALID_FOCUS;
	device->kii.MapDevice	= event_map_kii;
	device->kii.UnmapDevice	= event_unmap_kii;
	device->kii.HandleEvent	= event_handle_event;
	device->kii.event_mask	= KII_EM_KEY | KII_EM_POINTER;
	device->kii.priv.priv_ptr = device;

	if (KII_EOK != (err = kii_register_device(&device->kii, device_id))) {

		KRN_DEBUG(1, "failed to register kii_device (index %i)",
			device_id);
		kfree(device);
		return err;
	}
	KRN_DEBUG(2, "registered kii_device %i on focus %i", 
		device->kii.id, device->kii.focus_id);

	device->cmd_in_buffer  = kmalloc(EVENT_MAX_IO_BUF_SIZE, GFP_KERNEL);
	device->cmd_out_buffer = kmalloc(EVENT_MAX_IO_BUF_SIZE, GFP_KERNEL);
	if ((NULL == device->cmd_in_buffer) ||
		(NULL == device->cmd_out_buffer)) {

		kfree(device->cmd_in_buffer);
		kfree(device->cmd_out_buffer);
		kfree(device);
		return -ENOMEM;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	device->cmd_mutex = MUTEX;
#else
	init_MUTEX(&device->cmd_mutex);
#endif

	event_dev[device_id].pid = current->pid;
	event_dev[device_id].gid = current->gid;
	event_dev[device_id].ptr = device;
	event_dev[device_id].cnt++;

	KRN_DEBUG(1, "event_device %i initialized", device_id);


	return KII_EOK;
}

static void event_device_done(int device_id)
{
	KRN_ASSERT(device_id >= 0);

	if (--event_dev[device_id].cnt) {

		KRN_DEBUG(1, "event_device %i closed", device_id);
		return;
	}

	if (event_dev[device_id].ptr->kii.flags & KII_DF_FOCUSED) {

		kii_unmap_device(event_dev[device_id].ptr->kii.id);
	}
	kii_unregister_device(&(event_dev[device_id].ptr->kii));

	kfree(event_dev[device_id].ptr->cmd_in_buffer);
	kfree(event_dev[device_id].ptr->cmd_out_buffer);
	event_dev[device_id].ptr->cmd_in_buffer = NULL;
	event_dev[device_id].ptr->cmd_out_buffer = NULL;

	kfree(event_dev[device_id].ptr);
	event_dev[device_id].ptr = NULL;

	KRN_ASSERT(event_dev[device_id].cnt == 0);
	KRN_ASSERT(event_dev[device_id].ptr == NULL);
	KRN_ASSERT(event_dev[device_id].pid == 0);
	KRN_ASSERT(event_dev[device_id].gid == 0);

	KRN_DEBUG(1, "event_device %i finally closed", device_id);
}


static int event_open(struct inode *inode, struct file *kfile)
{
	event_file_t *file;
	void *queue_buffer;
	int err, device_id = event_device_id(inode->i_rdev);

	KRN_DEBUG(1, "open() event device %i", device_id);

	if (device_id < 0) {

		KRN_DEBUG(1, "open() failed: invalid device %x", inode->i_rdev);
		return -ENXIO;
	}

	if ((err = event_device_init(device_id))) {
		
		return err;
	}

	file =  kmalloc(sizeof(*file), GFP_KERNEL);
	queue_buffer = kmalloc(EVENT_QUEUE_SIZE, GFP_KERNEL);
	if ((NULL == file) || (NULL == queue_buffer)) {
	
		KRN_DEBUG(1, "failed to allocate event_file and queue_buffer");
		kfree(queue_buffer);
		kfree(file);
		event_device_done(device_id);
		return -ENOMEM;
	}
	memset(file, 0, sizeof(*file));
	file->refcnt++;
	file->device = event_dev[device_id].ptr;
	file->device_id = device_id;
	if (event_dev[device_id].pid == current->pid) {

		file->flags |= EVENT_FF_SESSION_LEADER;
	}
	file->queue.buffer = queue_buffer;
	file->queue.size = EVENT_QUEUE_SIZE;

	file->next = event_dev[device_id].ptr->files;
	event_dev[device_id].ptr->files = file;

	kfile->private_data = file;
	return KII_EOK;
}

static int event_release(struct inode *inode, struct file *kfile)
{
	event_file_t *file;
	int device_id;

	if (! (kfile && kfile->private_data)) {

		KRN_DEBUG(1, "event_release() on closed file?!");
		return -EINVAL;
	}
	file = (event_file_t *) kfile->private_data;
	device_id = file->device_id;

	KRN_DEBUG(1, "closing event device %i (refcnt %i)", 
		device_id, event_dev[device_id].cnt);

	if (event_dev[device_id].ptr->files == file) {

		event_dev[device_id].ptr->files = file->next;

	} else {

		event_file_t *prev = event_dev[device_id].ptr->files;

		while (prev->next != file) {

			prev = prev->next;
		}
		KRN_ASSERT(prev->next == file);

		prev->next = file->next;
	}
	file->next = NULL;

	kfile->private_data = NULL;
	file->refcnt--;
	file->device = NULL;
	file->device_id = -1;
	KRN_ASSERT(file->refcnt == 0);
	kfree(file->queue.buffer);
	file->queue.buffer = NULL;
	kfree(file);

	if (current->pid == event_dev[device_id].pid) {

		KRN_DEBUG(1, "session leader (pid %i) closed event_device %i",
			event_dev[device_id].pid, device_id);
		event_dev[device_id].pid = 0;
		event_dev[device_id].gid = 0;
	}

	event_device_done(device_id);
	return KII_EOK;
}

static struct file_operations event_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	owner:          THIS_MODULE,
#endif
	read:           event_read,
	poll:           event_poll,
	ioctl:          event_ioctl,
	open:           event_open,
	release:        event_release,
	fasync:		event_fasync
};


/*
**	Kernel Interface
*/
int dev_event_init(void)
{
	memset(&event_dev, 0, sizeof(event_dev));

	if (register_chrdev(EVENT_MAJOR, EVENT_NAME, &event_fops)) {

		printk(KERN_ERR "Can't register " EVENT_NAME " devices.\n");
		return -EBUSY;
	}
	printk(KERN_NOTICE EVENT_NAME " devices registered.\n");
	return KII_EOK;
}

#if defined(__MODULE__) || defined(MODULE)
int init_module(void)
{
	return dev_event_init();
}

void cleanup_module(void)
{
	unregister_chrdev(EVENT_MAJOR, EVENT_NAME);
}
#endif
