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
	__kgi_u32_t m = 1 << b;
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
