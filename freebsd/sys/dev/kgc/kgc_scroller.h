/*-
 * Copyright (c) 2003 Nicholas Souchu
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
 * KGI scroll generic implementation
 */

#ifndef _kgc_scroller_h
#define	_kgc_scroller_h

typedef struct scroller		*scroller_t;
typedef struct scroller_driver	scroller_driver_t;
#define scroller_method_t	kobj_method_t

struct scroller_driver {
	KOBJ_CLASS_FIELDS;
	scroller_driver_t	*previous;
};

struct scroller {
	/*
	 * A scroller is a kernel object. The first field must be the
	 * current ops table for the object.
	 */
	KOBJ_FIELDS;
	kgi_u_t		devid;
	kgi_console_t	*cons;
	void		*meta;
};

#define SCROLLMETHOD		KOBJMETHOD
#define SCROLLMETHOD_END	KOBJMETHOD_END

/*
 * At the declaration, propose the backdoor for early initialization,
 * when malloc is not yet ready.
 */
#define DECLARE_SCROLLER(name,driver,meta_type)					\
	static meta_type console_meta;						\
	static struct scroller console_scroller;				\
	static struct kobj_ops scroller_ops;					\
	void name##_configure(kgi_console_t *cons)				\
	{									\
	kobj_class_compile_static((kobj_class_t)&driver, &scroller_ops);	\
	kobj_init((kobj_t)&console_scroller, (kobj_class_t)&driver);		\
	console_scroller.meta = &console_meta;					\
	console_scroller.cons = cons;						\
	if (cons)								\
		cons->scroller = &console_scroller;				\
	kgc_scroller_register(&driver, 0, 1);					\
	kgc_scroller_register(&driver, 2, 1);					\
	kgc_scroller_alloc(0, &console_scroller);				\
	}

/*
 * Register the scroller from first to last consoles.
 */
extern void kgc_scroller_init(void);
extern scroller_t kgc_get_scroller(kgi_u_t devid);
extern scroller_t kgc_scroller_alloc(kgi_u_t devid, scroller_t s);
extern void kgc_scroller_release(kgi_u_t devid);
extern kgi_error_t kgc_scroller_register(scroller_driver_t *scroller, 
				 kgi_u_t display, kgi_u8_t already_allocated); 
extern kgi_error_t kgc_scroller_unregister(scroller_driver_t *driver);
extern void *kgc_scroller_meta(scroller_t s);
extern kgi_console_t *kgc_scroller_cons(scroller_t s);

/* XXX FIXME */
extern void textscroller_configure(kgi_console_t *cons);

#endif	/* !_kgc_scroller_h */
