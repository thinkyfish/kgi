/*-
 * Copyright (c) 2003-2004 Nicholas Souchu
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
 * KGI system layer defintion
 */

#ifndef	_KGI_SYSTEM_H_
#define	_KGI_SYSTEM_H_

#define EXPORT_SYMBOL(symbol)

#ifdef _KERNEL

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/kobj.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/condvar.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/kdb.h>

typedef struct {
	struct mtx mutex;
	struct cv  var;
} kgi_mutex_t;

typedef void *wait_queue_head_t;

#else /* _KERNEL */

#include <unistd.h>
#include <stdint.h>

#endif /* ! _KERNEL */

#include <dev/kgi/compiler.h>

#define MODULE_PARM(var,str)
#define KERN_ERR "kernel:"

#include <dev/kgi/types.h>

#ifdef KGI_SYS_NEED_USER

extern __kgi_u32_t kgi_copy_to_user(void *to, const void *from, __kgi_u32_t n);
extern __kgi_u32_t kgi_copy_from_user(void *to, const void *from, 
	__kgi_u32_t n);

#define put_user(x,ptr) kgi_copy_to_user(&x, ptr, sizeof(*ptr))
#define get_user(x,ptr) kgi_copy_from_user(&x, ptr, sizeof(*ptr))

#endif /* KGI_SYS_NEED_USER */

#ifdef KGI_SYS_NEED_VM

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>

#include <vm/kgi_pager.h>

#define virt_to_phys vtophys
#define virt_to_bus vtophys

extern __kgi_virt_addr_t kgi_map_buffer(__kgi_phys_addr_t paddr,
		 __kgi_size_t size);
extern void kgi_unmap_buffer(__kgi_virt_addr_t vaddr, __kgi_size_t size);

#endif /* KGI_SYS_NEED_VM */

#ifdef KGI_SYS_NEED_IO

#define	__KGI_SYS_IO_HAS_PCICFG
#define	__KGI_SYS_IO_HAS_BUS
#define	__KGI_SYS_IO_HAS_IO
#define	__KGI_SYS_IO_HAS_MEM
#define	__KGI_SYS_IO_HAS_IRQ

/* 
 * XXX I don't think all this stuff has to be there
 * XXX I'd rather put it in the system specific directory of graphic drivers.
 */

#include <sys/bus.h>
#include <dev/kgi/io.h>
#include <dev/kgi/pci.h>

#endif /* KGI_SYS_NEED_IO */

#ifdef KGI_SYS_NEED_ATOMIC

#include <machine/atomic.h>

typedef __kgi_u32_t kgi_atomic_t;

#define kgi_atomic_set(p, v) (*p=v)
#define kgi_atomic_read(p) (*p)

extern kgi_atomic_t kgi_test_and_set_bit32(__kgi_u32_t b, volatile void *p);
extern void kgi_clear_bit(int b, volatile void *p);
extern void kgi_set_bit(int b, volatile void *p);
extern int kgi_test_bit(int b, volatile void *p);
extern int kgi_find_first_zero_bit(volatile void *p, int max);

#endif /* KGI_SYS_NEED_ATOMIC */

#define kgi_udelay(d) DELAY((int)d)
#define kgi_nanosleep(x) kgi_udelay(2)

#ifdef KGI_SYS_NEED_PROC

#define kgi_wakeup(p) wakeup(p)
extern struct proc *kgiproc;

#endif /* KGI_SYS_NEED_PROC */

#ifdef KGI_SYS_NEED_MALLOC

#include <sys/malloc.h>

MALLOC_DECLARE(M_KGI);

extern void *kgi_kmalloc(__kgi_size_t size);
extern void kgi_kfree(const void *ptr);
extern void *kgi_cmalloc(__kgi_size_t size);
extern void kgi_cfree(const void *ptr, __kgi_size_t size);

#define kgim_alloc kgi_kmalloc
#define kgim_free kgi_kfree

#endif /* KGI_SYS_NEED_MALLOC */

#ifdef KGI_SYS_NEED_MUTEX

extern void kgi_mutex_init(kgi_mutex_t *mtx, const char *name);
extern void kgi_mutex_done(kgi_mutex_t *mtx);

#define kgi_mutex_lock(mtx) do { mtx_lock(&(mtx)->mutex); } while (0)
#define kgi_mutex_unlock(mtx) do { mtx_unlock(&(mtx)->mutex); } while (0)
#define kgi_mutex_assert(mtx,type) do { mtx_assert(&(mtx)->mutex, type); } while (0)

extern void kgi_mutex_wait(kgi_mutex_t *mtx);
extern void kgi_mutex_signal(kgi_mutex_t *mtx, 
		int unblock_all /* TRUE or FALSE */);

#define KGI_MUTEX_OWNED		MA_OWNED
#define KGI_MUTEX_NOTOWNED	MA_NOTOWNED
#define KGI_MUTEX_RECURSED	MA_RECURSED
#define KGI_MUTEX_NOTRECURSED	MA_NOTRECURSED

/* KGI Giant lock. */
extern kgi_mutex_t kgi_lock;

#endif /* KGI_SYS_NEED_MUTEX */

#endif	/* _KGI_SYSTEM_H_ */

