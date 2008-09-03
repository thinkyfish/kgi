/* ----------------------------------------------------------------------------
**	/dev/event resource mapper definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	
**	$FreeBSD$	
**	
*/
#ifndef _kii_event_h
#define	_kii_event_h

#define	EVENT_MAJOR	61
#define	EVENT_NAME	"event"

typedef struct event_file_s	event_file_t;
typedef struct event_device_s	event_device_t;

/*
**	A device represents the actual device.
*/
#define	EVENT_DEVICE_SIZE	PAGE_SIZE

#define	EVENT_QUEUE_SIZE	PAGE_SIZE

struct event_device_s
{
	kii_u_t		id;
	kii_device_t	kii;
	struct mtx	cmd_mutex;
	event_file_t	*files;
};

/*
**	A file is a particular virtual view of a device.
*/
typedef enum
{
	EVENT_FF_CLIENT_IDENTIFIED	= 0x00000001,
	EVENT_FF_SESSION_LEADER		= 0x00000002,
	EVENT_FF_QUEUE_OVERRUN		= 0x00000004

} event_file_flags_t;

struct event_file_s
{
	event_file_t	*next;

	unsigned long refcnt;
	event_file_flags_t	flags;

	kii_u_t		device_id;
	kii_u_t		previous;

	event_device_t	*device;

	struct	{
		kii_u_t		head, tail, size;
		kii_u8_t	*buffer;
	}		queue;
};

#endif	/* #ifndef _kii_event_h */
