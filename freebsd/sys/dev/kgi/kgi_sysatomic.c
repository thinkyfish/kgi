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
 * KGI FreeBSD atomic system layer
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_ATOMIC
#include <dev/kgi/system.h>

kgi_atomic_t
kgi_test_and_set_bit32(__kgi_u32_t b, volatile void *p)
{
	int s = splhigh();
	__kgi_u32_t m = 1<<b;
	__kgi_u32_t r = *(volatile __kgi_u32_t *)p & m;
	*(volatile __kgi_u32_t *)p |= m;
	splx(s);
	return (r);
}

void
kgi_clear_bit(int b, volatile void *p)
{

    atomic_clear_int(((volatile int *)p) + (b >> 5), 1 << (b & 0x1f));
}

void
kgi_set_bit(int b, volatile void *p)
{

    atomic_set_int(((volatile int *)p) + (b >> 5), 1 << (b & 0x1f));
}

int
kgi_test_bit(int b, volatile void *p)
{

    return (((volatile int *)p)[b >> 5] & (1 << (b & 0x1f)));
}

int
kgi_find_first_zero_bit(volatile void *p, int max)
{
    int b;

    for (b = 0; b < max; b += 32) {
	if (((volatile int *)p)[b >> 5] != ~0) {
	    for (;;) {
		if ((((volatile int *)p)[b >> 5] & (1 << (b & 0x1f))) == 0)
		    return (b);
		b++;
	    }
	}
    }
    return (max);
}
