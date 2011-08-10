/*-
 * Copyright (c) 1998-2000 Steffen Seeger
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
 * Intel style in/out I/O space.
 */

#define KGI_SYS_NEED_IO
#include <dev/kgi/x86-compiler.h>

__inline __kgi_u8_t
io_in8(const io_vaddr_t addr)
{
	__kgi_u8_t  val;
	__asm__ __volatile__ ("inb %w1,%0" : "=a" (val) : "Nd" (addr));

	return (val);
}

__inline __kgi_u16_t
io_in16(const io_vaddr_t addr)
{
	__kgi_u16_t val;
	__asm__ __volatile__ ("inw %w1,%0" : "=a" (val) : "Nd" (addr));

	return (val);
}

__inline __kgi_u32_t
io_in32(const io_vaddr_t addr)
{
	__kgi_u32_t val;
	__asm__ __volatile__ ("inl %w1,%0" : "=a" (val) : "Nd" (addr));

	return (val);
}

__inline void
io_out8(const __kgi_u8_t val, const io_vaddr_t addr)
{

	__asm__ __volatile__ ("outb %b0,%w1" : : "a" (val), "Nd" (addr));
}

__inline void
io_out16(const __kgi_u16_t val, const io_vaddr_t addr)
{

	__asm__ __volatile__ ("outw %w0,%w1" : : "a" (val), "Nd" (addr));
}

__inline void
io_out32(const __kgi_u32_t val, io_vaddr_t addr)
{

	__asm__ __volatile__ ("outl %0,%w1" : : "a" (val), "Nd" (addr));
}

__inline void
io_ins8(const io_vaddr_t addr, void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; insb"
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

__inline void
io_ins16(const io_vaddr_t addr, void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; insw"
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

__inline void
io_ins32(const io_vaddr_t addr, void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; insl"
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

__inline void
io_outs8(const io_vaddr_t addr, const void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; outsb"
		: "=S" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

__inline void
io_outs16(const io_vaddr_t addr, const void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; outsw"
		: "=S" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

__inline void
io_outs32(const io_vaddr_t addr, const void *buf, __kgi_size_t cnt)
{

	__asm__ __volatile__ ("cld ; rep ; outsl"
		: "=S" (buf), "=c" (cnt) : "d" (addr),"0" (buf), "1" (cnt));
}

/*
 * Memory I/O space
 */
__DECLARE_MEM_OPS(8)
__DECLARE_MEM_OPS(16)
__DECLARE_MEM_OPS(32)

