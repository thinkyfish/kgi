/* ----------------------------------------------------------------------------
**	hardware I/O layer definiton
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: io.h,v $
*/
#ifndef	_kgi_io_h
#define	_kgi_io_h

/*	Explanations on I/O regions can be found in the system layer 
**	documentation.
*/

/* ----------------------------------------------------------------------------
**	PCI configuration space
** ----------------------------------------------------------------------------
**	These functions should be used to access the PCI configuration space.
**	As the address is given by the hardware wiring, a registration
**	scheme doesn't make sense. But I/O is pretty useful :) For the same
**	reason, and as it is used for configuration (bootstrap) purposes,
**	a physical->virtual mapping is not possible. All addresses are
**	virtual and no mapping takes place.
*/
#ifdef __KGI_SYS_IO_HAS_PCICFG

#define	PCICFG_NULL ((pcicfg_vaddr_t) 0xFFFF0000)	/* an invalid virtual address	*/

typedef __kgi_u32_t	pcicfg_vaddr_t;	/* the virtual address type	*/

#define	PCICFG_VADDR(bus, slot, fn)	\
	((pcicfg_vaddr_t)((((bus) & 0xFF) << 24) | (PCI_DEVFN((slot),(fn)) << 16)))
#define	PCICFG_BUS(vaddr)	(((vaddr) >> 24) & 0xFF)
#define	PCICFG_DEV(vaddr)	PCI_SLOT(((vaddr) >> 16) & 0xFF)
#define	PCICFG_FN(vaddr)	PCI_FUNC(((vaddr) >> 16) & 0xFF)
#define	PCICFG_REG(vaddr)	((vaddr) & 0xFFFF)
#define	PCICFG_SIGNATURE(vendor, device) ((vendor << 16) | device)

extern int pcicfg_find_device(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures);
extern int pcicfg_find_subsystem(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures);

extern __kgi_u8_t  pcicfg_in8 (const pcicfg_vaddr_t vaddr);
extern __kgi_u16_t pcicfg_in16(const pcicfg_vaddr_t vaddr);
extern __kgi_u32_t pcicfg_in32(const pcicfg_vaddr_t vaddr);

extern void pcicfg_out8 (const __kgi_u8_t  val, const pcicfg_vaddr_t vaddr);
extern void pcicfg_out16(const __kgi_u16_t val, const pcicfg_vaddr_t vaddr);
extern void pcicfg_out32(const __kgi_u32_t val, const pcicfg_vaddr_t vaddr);

#define	PCICFG_SET_BASE32(value, reg)		\
		pcicfg_out32(0xFFFFFFFF, reg);	\
		pcicfg_in32(reg);		\
		pcicfg_out32((value), reg)

#endif	/* #ifdef __KGI_SYS_IO_HAS_PCICFG	*/


/* ----------------------------------------------------------------------------
**	Intel style in/out I/O space
** ----------------------------------------------------------------------------
**	The io_in.. / io_out.. functions generate the neccesary hardware
**	actions to do a read/write cycle in a cards I/O space. (Registers
**	accessed via the in.. / out.. instructions on i386 architecture.)
*/
#ifdef __KGI_SYS_IO_HAS_IO

#define	IO_NULL	((io_vaddr_t) 0)	/* an invalid virtual address	*/
#define	IO_DECODE_ALL ((io_addr_t) -1)	/* all lines being decoded	*/

typedef __kgi_u_t	io_addr_t;	/* the physical address type	*/
typedef __kgi_u_t	io_vaddr_t;	/* the virtual address type	*/

typedef struct
{
	pcicfg_vaddr_t	device;		/* (PCI) device this belongs to	*/
	io_vaddr_t	base_virt;	/* virtual base address		*/
	io_addr_t	base_io;	/* I/O base address		*/
	io_addr_t	base_bus;	/* bus address			*/
	io_addr_t	base_phys;	/* physical address		*/
	io_addr_t	size;		/* size of region		*/
	io_addr_t	decode;		/* decoded I/O address lines	*/
	const __kgi_ascii_t *name;	/* name of the region		*/

} io_region_t;

extern int 	  io_check_region(io_region_t *r);
extern io_vaddr_t io_claim_region(io_region_t *r);
extern io_vaddr_t io_free_region(io_region_t *r);
extern io_vaddr_t io_alloc_region(io_region_t *r);

extern __kgi_u8_t  io_in8 (const io_vaddr_t vaddr);
extern __kgi_u16_t io_in16(const io_vaddr_t vaddr);
extern __kgi_u32_t io_in32(const io_vaddr_t vaddr);

extern void io_out8 (const __kgi_u8_t  val, const io_vaddr_t vaddr);
extern void io_out16(const __kgi_u16_t val, const io_vaddr_t vaddr);
extern void io_out32(const __kgi_u32_t val, const io_vaddr_t vaddr);

extern void io_ins8 (const io_vaddr_t addr, void *buf, __kgi_size_t cnt);
extern void io_ins16(const io_vaddr_t addr, void *buf, __kgi_size_t cnt);
extern void io_ins32(const io_vaddr_t addr, void *buf, __kgi_size_t cnt);

extern void io_outs8 (const io_vaddr_t addr, const void *buf, __kgi_size_t cnt);
extern void io_outs16(const io_vaddr_t addr, const void *buf, __kgi_size_t cnt);
extern void io_outs32(const io_vaddr_t addr, const void *buf, __kgi_size_t cnt);

#endif	/* #ifdef __KGI_SYS_IO_HAS_IO	*/

/* ----------------------------------------------------------------------------
**	memory I/O space
** ----------------------------------------------------------------------------
**	This is the 'normal' shared memory I/O space accessed using the
**	mov instructions on i386 architecture. The difference between
**	*vaddr = val and mem_out32(val, vaddr) is that the latter will not
**	be optimized away when compiling with maximum optimization.
*/
#ifdef __KGI_SYS_IO_HAS_MEM

#define	MEM_NULL	((mem_vaddr_t) 0) /* an invalid virtual address	*/
#define	MEM_DECODE_ALL	((mem_iaddr_t) -1UL) /* all lines being decoded	*/

typedef __kgi_phys_addr_t mem_paddr_t;	/* the physical address type	*/
typedef __kgi_virt_addr_t mem_vaddr_t;	/* the virtual address type	*/
typedef	__kgi_bus_addr_t  mem_baddr_t;	/* the bus address type		*/
typedef	__kgi_phys_addr_t mem_iaddr_t;	/* the decoder address type	*/

typedef struct
{
	pcicfg_vaddr_t	device;		/* (PCI) device this belongs to	*/
	mem_vaddr_t	base_virt;	/* virtual base address		*/
	mem_iaddr_t	base_io;	/* the I/O base address		*/
	mem_baddr_t	base_bus;	/* bus address			*/
	mem_paddr_t	base_phys;	/* physical address		*/
	__kgi_size_t	size;		/* size of region		*/
	mem_iaddr_t	decode;		/* decoded io address lines	*/
	const __kgi_ascii_t *name;	/* name of the region		*/

} mem_region_t;

extern int	   mem_check_region(mem_region_t *r);
extern mem_vaddr_t mem_claim_region(mem_region_t *r);
extern mem_vaddr_t mem_free_region(mem_region_t *r);
extern mem_vaddr_t mem_alloc_region(mem_region_t *r);

extern __kgi_u8_t  mem_in8 (const mem_vaddr_t vaddr);
extern __kgi_u16_t mem_in16(const mem_vaddr_t vaddr);
extern __kgi_u32_t mem_in32(const mem_vaddr_t vaddr);

extern void mem_out8 (const __kgi_u8_t  val, const mem_vaddr_t vaddr);
extern void mem_out16(const __kgi_u16_t val, const mem_vaddr_t vaddr);
extern void mem_out32(const __kgi_u32_t val, const mem_vaddr_t vaddr);

extern void mem_ins8 (const mem_vaddr_t addr, void *buf, __kgi_size_t cnt);
extern void mem_ins16(const mem_vaddr_t addr, void *buf, __kgi_size_t cnt);
extern void mem_ins32(const mem_vaddr_t addr, void *buf, __kgi_size_t cnt);

extern void mem_outs8 (const mem_vaddr_t addr, const void *buf, __kgi_size_t cnt);
extern void mem_outs16(const mem_vaddr_t addr, const void *buf, __kgi_size_t cnt);
extern void mem_outs32(const mem_vaddr_t addr, const void *buf, __kgi_size_t cnt);

extern void mem_put8 (const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);
extern void mem_put16(const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);
extern void mem_put32(const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);

extern void mem_get8 (void *buf, const mem_vaddr_t src, __kgi_size_t cnt);
extern void mem_get16(void *buf, const mem_vaddr_t src, __kgi_size_t cnt);
extern void mem_get32(void *buf, const mem_vaddr_t src, __kgi_size_t cnt);

#endif	/* #ifdef __KGI_SYS_IO_HAS_MEM	*/

/* ----------------------------------------------------------------------------
**	irq handling
** ----------------------------------------------------------------------------
**	use similar to io regions.
*/
#ifdef __KGI_SYS_IO_HAS_IRQ

#define	IRQ_NULL	((unsigned int) -1)	/* an invalid irq line	*/
#define	IRQ_DECODE_ALL	((irq_mask_t) -1)	/* all lines decoded	*/

typedef __kgi_u_t	irq_mask_t;

typedef enum
{
	IF_SHARED_IRQ	= 0x00000001
	
} irq_flags_t;

typedef struct irq_system_s irq_system_t; /* system dependent data */
typedef int irq_handler_fn(void *meta, void *meta_io, irq_system_t *system);

typedef struct
{
	irq_flags_t		flags;	/* properties			*/
	const __kgi_ascii_t	*name;	/* name of the line		*/
	__kgi_u_t		line;	/* requested IRQ line		*/

	void	*meta;			/* handler meta data		*/
	void	*meta_io;		/* handler meta I/O data	*/
	irq_handler_fn	*High, *Low;	/* high/low priority handlers	*/

} irq_line_t;

/*	These may be OS dependent, and have to be supplied by the OS/kernel
**	layer. As they aren't performance critical either, a call doesn't hurt.
*/
extern __kgi_error_t  irq_claim_line(irq_line_t *irq);
extern void irq_free_line(irq_line_t *irq);

#endif	/* #ifdef __KGI_SYS_IO_HAS_IRQ	*/

#undef	__KGI_SYS_IO_HAS_PCICFG
#undef	__KGI_SYS_IO_HAS_IO
#undef	__KGI_SYS_IO_HAS_MEM
#undef	__KGI_SYS_IO_HAS_IRQ

#endif	/* #ifdef _kgi_io_h */
