/*-
 * Copyright (C) 2004 Nicholas Souchu
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
 * KGI FreeBSD mutex system layer
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MUTEX
#include <dev/kgi/system.h>

#include <dev/kgi/kgi.h>

void
kgi_mutex_init(kgi_mutex_t *mtx, const char *name)
{

	mtx_init(&mtx->mutex, name, NULL, MTX_DEF);
	cv_init(&mtx->var, name);
}

void
kgi_mutex_done(kgi_mutex_t *mtx)
{

	cv_destroy(&mtx->var);
	mtx_destroy(&mtx->mutex);
}

/*
 * Wait on the mutex e.g unlock the mutex
 * and add the thread to the list of waiting
 * threads on this mutex. When the thread is
 * made runnable again, it owns the mutex.
 *
 * Important: the mutex must be already locked by
 * the current running thread when calling
 * kgi_mutex_wait().
 */
void
kgi_mutex_wait(kgi_mutex_t *mtx)
{

	cv_wait(&mtx->var, &mtx->mutex);
}

/*
 * Wakeup (unblock) threads waiting on the mutex.
 *
 * If unblock_all is TRUE then unblock all waiters
 * otherwise only one.
 */
void
kgi_mutex_signal(kgi_mutex_t *mtx, int unblock_all)
{

	if (unblock_all)
		cv_broadcast(&mtx->var);
	else
		cv_signal(&mtx->var);
}
