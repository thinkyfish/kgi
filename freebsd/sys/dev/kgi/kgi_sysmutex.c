/*-
 * Copyright (c) 2004 Nicholas Souchu
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
