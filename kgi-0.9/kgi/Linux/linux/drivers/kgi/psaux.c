/* ----------------------------------------------------------------------------
**	PS/2 auxiliary device file emulation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**
**	$Log: psaux.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/version.h>

#include <linux/kgii.h>
#include <kgi/debug.h>
#include <kii/kii.h>
#include "psaux.h"

#define	PSAUX_BUF_SIZE	512	/* buffer size; must be a power of 2	*/
#define	PSAUX_BUF_MASK	(PSAUX_BUF_SIZE - 1)

#define	EOK	0

static struct psaux_queue
{
	kii_u_t			buttons;	/* button state		*/
	kii_s_t			x,y;		/* last position	*/

	kii_u_t			head;
	kii_u_t			tail;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
	struct wait_queue	*proc_list;
#else
	struct wait_queue_head_t	*proc_list;
#endif
	struct fasync_struct	*fasync;
	kii_u8_t			buf[PSAUX_BUF_SIZE];

} psaux_queue[CONFIG_KGII_MAX_NR_FOCUSES];

/*	put a motion/button change packet into the queue
*/
static int psaux_queue_packet(struct psaux_queue *queue, const kii_u8_t *buf)
{
	if ((queue->head - queue->tail) > (PSAUX_BUF_SIZE-3)) {

		return -ENOMEM;
	}

	queue->buf[queue->head++ & PSAUX_BUF_MASK] = buf[0];
	queue->buf[queue->head++ & PSAUX_BUF_MASK] = buf[1];
	queue->buf[queue->head++ & PSAUX_BUF_MASK] = buf[2];

	if (queue->fasync) {

		kill_fasync(queue->fasync, SIGIO);
	}
	wake_up_interruptible(&queue->proc_list);

	return EOK;
}

void psaux_handle_kii_event(kii_event_t *ev)
{
	struct psaux_queue	*queue;
	kii_u8_t		packet[4];

	if (((1 << ev->any.type) & ~KII_EM_POINTER) ||
		! KII_VALID_FOCUS_ID(ev->any.focus)) {

		return;
	}

	queue = psaux_queue + ev->any.focus;

	switch (ev->any.type) {

	case KII_EV_PTR_RELATIVE:
		ev->pmove.y = -ev->pmove.y;
		packet[0] = 0x08 | (queue->buttons & 0x07);
		if (ev->pmove.x < 0)	packet[0] |= 0x10;
		if (ev->pmove.y < 0)	packet[0] |= 0x20;
		packet[1] = ev->pmove.x & 0xFF;
		packet[2] = ev->pmove.y & 0xFF;

		if (psaux_queue_packet(queue, packet) == EOK) {

			queue->x += ev->pmove.x;
			queue->y += ev->pmove.y;
		}
		return;

	case KII_EV_PTR_ABSOLUTE:
		return;

	case KII_EV_PTR_BUTTON_PRESS:
	case KII_EV_PTR_BUTTON_RELEASE:
		ev->pbutton.state &= 0x07;
		packet[0] = 0x08 | ev->pbutton.state;
		packet[1] = packet[2] = 0x00;
		if ((ev->pbutton.state != queue->buttons) && 
			(psaux_queue_packet(queue, packet) == EOK)) {

			queue->buttons = ev->pbutton.state;
		}
		return;

	default:
		KRN_INTERNAL_ERROR;
	}
}

/*
**	The PSAUX device file operations.
*/

static int psaux_fasync(int fd, struct file *file, int on)
{
	struct psaux_queue *queue = file->private_data;
	int retval = fasync_helper(fd, file, on, &queue->fasync);
	return (retval < 0) ? retval : EOK;
}

static int psaux_open(struct inode *inode, struct file *file)
{
	int focus = kii_focus_of_task(current); 

	if (focus < 0) {

		return -EINVAL;
	}

	file->private_data = psaux_queue + focus;
	return EOK;
}

static int psaux_close(struct inode *inode, struct file *file)
{
	psaux_fasync(-1, file, 0);
	file->private_data = NULL;
	return -EINVAL;
}

static ssize_t psaux_read(struct file *file, char *buffer, 
	size_t count, loff_t *ppos)
{
	struct psaux_queue *queue = file->private_data;
	kii_u_t retval;

	KRN_ASSERT(queue);

	if (queue->head == queue->tail) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
		struct wait_queue wait = { current, NULL };
#else
		DECLARE_WAITQUEUE(wait, current);
#endif
		if (file->f_flags & O_NONBLOCK) {

			return -EAGAIN;
		}
		add_wait_queue(&queue->proc_list, &wait);
repeat:		current->state = TASK_INTERRUPTIBLE;
		if ((queue->head == queue->tail) &&
			! signal_pending(current)) {

			schedule();
			goto repeat;
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&queue->proc_list, &wait);
	}

	if (queue->head - queue->tail < count) {

		count = queue->head - queue->tail;
	}
	retval = count;
	while (count--) {

		if (put_user(queue->buf[queue->tail], buffer)) {

			return -EFAULT;
		}
		buffer++;
		queue->tail++;
		if (queue->tail < PSAUX_BUF_SIZE) {

			continue;
		}
		KRN_ASSERT(queue->tail <= queue->head);
		queue->tail -= PSAUX_BUF_SIZE;
		queue->head -= PSAUX_BUF_SIZE;
	}
	return retval;
}

static ssize_t psaux_write(struct file *file, const char *buffer, 
	size_t count, loff_t *ppos)
{
	/* simulate write !!! */
	return -EIO;
}

static unsigned int psaux_poll(struct file *file, poll_table *wait)
{
	struct psaux_queue *queue = file->private_data;

	KRN_ASSERT(queue);

	poll_wait(file, &queue->proc_list, wait);
	return (queue->head == queue->tail) ? 0 : POLLIN | POLLRDNORM;
}

static struct file_operations psaux_fops =
{
	NULL,		/* seek		*/
	psaux_read,	/* read		*/
	psaux_write,	/* write	*/
	NULL,		/* readdir	*/
	psaux_poll,	/* poll		*/
	NULL,		/* ioctl	*/
	NULL,		/* mmap		*/
	psaux_open,	/* open		*/
	NULL,		/* flush	*/
	psaux_close,	/* release	*/
	NULL,		/* 		*/
	psaux_fasync	/* fasync	*/
};

struct miscdevice psaux_device =
{
	PSMOUSE_MINOR, "psaux", &psaux_fops
};

void psaux_init(void)
{
	memset(psaux_queue, 0, sizeof(psaux_queue));
	KRN_NOTICE("PS/2 auxiliary device emulation driver");
	misc_register(&psaux_device);
}
