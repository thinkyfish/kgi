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
 * x86 compiler dependent stuffitude.
 */

#ifndef	_KGI_X86_COMPILER_H_
#define	_KGI_X86_COMPILER_H_

#define __KGI_SYS_IO_HAS_IO
#define __KGI_SYS_IO_HAS_BUS
#include <dev/kgi/system.h>
#include <dev/kgi/types.h>

#ifdef KGI_SYS_NEED_IO
inline __kgi_u8_t io_in8(const io_vaddr_t addr);
inline __kgi_u16_t io_in16(const io_vaddr_t addr);
inline __kgi_u32_t io_in32(const io_vaddr_t addr);
inline void io_out8(const __kgi_u8_t val, const io_vaddr_t addr);
inline void io_out16(const __kgi_u16_t val, const io_vaddr_t addr);
inline void io_out32(const __kgi_u32_t val, io_vaddr_t addr);
inline void io_ins8(const io_vaddr_t addr, void *buf, __kgi_size_t cnt);
inline void io_ins16(const io_vaddr_t addr, void *buf, __kgi_size_t cnt);
inline void io_ins32(const io_vaddr_t addr, void *buf, __kgi_size_t cnt);
inline void io_outs8(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt);
inline void io_outs16(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt);
inline void io_outs32(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt);

/*
 * memory I/O space
 */
#define	__DECLARE_MEM_OPS(w)						\
	extern __inline__ __kgi_u##w##_t mem_in##w (mem_vaddr_t addr) 	\
	{								\
		return *((__volatile__ __kgi_u##w##_t *) addr);		\
	}								\
	extern __inline__ void mem_out##w (const __kgi_u##w##_t val,	\
		const mem_vaddr_t addr)					\
	{								\
		*((__volatile__ __kgi_u##w##_t *) addr) = val;		\
	}								\
	extern __inline__ void mem_ins##w (const mem_vaddr_t addr,	\
		void *buf, __kgi_size_t cnt)				\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			((__volatile__ __kgi_u##w##_t *) buf)[i++] =	\
				*((__volatile__ __kgi_u##w##_t *) addr);\
		}							\
	}								\
	extern __inline__ void mem_outs##w (const mem_vaddr_t addr, 	\
		const void *buf, __kgi_size_t cnt)			\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			*((__volatile__ __kgi_u##w##_t *) addr) =	\
			   ((__volatile__ __kgi_u##w##_t *) buf)[i++];  \
		}							\
	}								\
	extern __inline__ void mem_put##w (const mem_vaddr_t addr,	\
		const void *buf, __kgi_size_t cnt)			\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			((__volatile__ __kgi_u##w##_t *) addr)[i] =	\
			   ((__volatile__ __kgi_u##w##_t *) buf)[i];	\
			i++;						\
		}							\
	}

#endif /* !KGI_SYS_NEED_IO */
#endif /* !_KGI_X86_COMPILER_H_ */
