#ifndef _compiler_h
#define	_compiler_h

#if	__GNUC__ > 2 || __GNUC_MINOR__ >= 91

struct __unaligned_u64 { unsigned long  value __attribute__((packed)); };
struct __unaligned_u32 { unsigned int   value __attribute__((packed)); };
struct __unaligned_u16 { unsigned short value __attribute__((packed)); };

static inline unsigned long ldl_u(void *p)
{
	const struct __unaligned_u32 *ptr = (const struct __unaligned_u32 *) p;
	return ptr->value;
}

#else
#error	need to implement unaligned loads ldl_u() for your compiler!
#endif


#endif	/* #ifdef _compiler_h	*/
