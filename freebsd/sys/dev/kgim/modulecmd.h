/*-
 * Copyright (c) 1999-2000 Steffen Seeger
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
 * KGI module specific command requests
 */


#ifndef _kgi_modulecmd_h
#define	_kgi_modulecmd_h

struct kgic_img_buffer_param_t {
	kgi_u_t		index;		/* index of buffer		*/
};

struct kgic_img_buffer_result_t {
	kgi_u_t		mmio;		/* mmio_region index to use	*/
	kgi_u_t		mmio_offset;	/* offset into region (bytes)	*/
	kgi_u_t		accel;		/* accelerator to use		*/

	kgi_scoord_t	size;		/* application area size	*/
	kgi_scoord_t	origin;		/* application area origin	*/
	kgi_u_t		stride;		/* application area stride	*/

	kgi_u_t	lpm[__KGI_MAX_NR_ATTRIBUTES];	/* left pixel masks	*/
	kgi_u_t	rpm[__KGI_MAX_NR_ATTRIBUTES];	/* right pixel masks	*/
	kgi_u_t	cpm[__KGI_MAX_NR_ATTRIBUTES];	/* common pixel masks	*/
};

#endif /* #ifdef _kgi_modulecmd_h */
