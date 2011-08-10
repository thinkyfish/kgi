/*-
 * Copyright (c) 2000 Steffen Seeger
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
 * /dev/event resource mapper definition
 */

#ifndef _KII_EVENT_H_
#define	_KII_EVENT_H_

#define	EVENT_MAJOR	61
#define	EVENT_NAME	"event"

typedef struct event_file_s	event_file_t;
typedef struct event_device_s	event_device_t;

/*
 * A device represents the actual device.
 */
#define	EVENT_DEVICE_SIZE	PAGE_SIZE
#define	EVENT_QUEUE_SIZE	PAGE_SIZE

struct event_device_s {
	kii_u_t		id;
	kii_device_t	kii;
	struct mtx	cmd_mutex;
	event_file_t	*files;
};

/*
 * A file is a particular virtual view of a device.
 */
typedef enum {
	EVENT_FF_CLIENT_IDENTIFIED	= 0x00000001,
	EVENT_FF_SESSION_LEADER		= 0x00000002,
	EVENT_FF_QUEUE_OVERRUN		= 0x00000004
} event_file_flags_t;

struct event_file_s {
	event_file_t		*next;
	unsigned long 		refcnt;
	event_file_flags_t	flags;
	kii_u_t			device_id;
	kii_u_t			previous;
	event_device_t 		*device;
	struct	{
		kii_u_t		head;
		kii_u_t		tail;
		kii_u_t		size;
		kii_u8_t	*buffer;
	} queue;
};

#endif /* !_KII_EVENT_H_ */
