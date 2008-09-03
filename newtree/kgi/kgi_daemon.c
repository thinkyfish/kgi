/* ----------------------------------------------------------------------------
**	KGI daemon
** ----------------------------------------------------------------------------
**	Copyright (C)	2004	Nicholas Souchu
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
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_PROC
#define KGI_SYS_NEED_MUTEX
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

static void kgi_daemon(void);

struct proc *kgiproc;

static struct kproc_desc kgi_kp = {
	"kgidaemon",
	kgi_daemon,
	&kgiproc
};
SYSINIT(kgidaemon, SI_SUB_KTHREAD_KGI, SI_ORDER_FIRST, kproc_start, &kgi_kp)

void kgi_daemon(void)
{
	kgi_mutex_lock(&kgi_lock);
	while (TRUE) {
		msleep(&kgiproc, &kgi_lock.mutex, PZERO /* XXX */,
		       "kdsleep", hz /* 1s */);

		/* Poll the keyboard events needing system action */
		kii_bottomhalf();
	}
}
