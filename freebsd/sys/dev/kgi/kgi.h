/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2002-2004 Nicholas Souchu
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
 * KGI manager OS kernel independent stuff
 */

#ifndef _KGI_KGI_H_
#define _KGI_KGI_H_

/*
 * kgi.h is now just a set of includes. Each of them
 * has its own rule of #ifdef _KERNEL depending on
 * their respective purpose.
 */

#include <dev/kgi/system.h>

KGI_SYS_DECLARE_INTEGRAL_TYPES(kgi)

#include <dev/kgi/kgierr.h>
#include <dev/kgi/kgidbg.h>
#include <dev/kgi/kgityp.h>
#include <dev/kgi/kgimod.h>
#include <dev/kgi/kgires.h>
#include <dev/kgi/kgidpy.h>
#include <dev/kgi/kgicmd.h>

#endif
