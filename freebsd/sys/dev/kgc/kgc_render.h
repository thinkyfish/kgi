/*-
 * Copyright (C) 2003 Nicholas Souchu
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
 * KGI console implementation specific definitions
 */

#ifndef _kgc_render_h
#define	_kgc_render_h

#define KGI_RF_NO_HARDSCROLL	0x00000001
#define KGI_RF_TILE_Y		0x00000002
#define KGI_RF_NEEDS_UPDATE	0x00000004

typedef struct render		*render_t;
typedef struct render_driver	render_driver_t;
#define render_method_t	kobj_method_t

struct render_driver {
	KOBJ_CLASS_FIELDS;
	render_driver_t	*previous;
};

#define RENDERMETHOD		KOBJMETHOD
#define RENDERMETHOD_END	KOBJMETHOD_END

struct render {
	/*
	 * A render is a kernel object. The first field must be the
	 * current ops table for the object.
	 */
	KOBJ_FIELDS;

	kgi_u_t		devid;
	kgi_console_t	*cons;
	void		*meta;
};

/*
 * At the declaration, propose the backdoor for early initialization,
 * when malloc is not yet ready.
 */
#define DECLARE_RENDER(name,driver,meta_type)					\
	static meta_type console_meta;						\
	static struct render console_render;					\
	static struct kobj_ops render_ops;					\
	void name##_configure(kgi_console_t *cons)				\
	{									\
		kobj_class_compile_static((kobj_class_t)&driver, &render_ops);	\
		kobj_init((kobj_t)&console_render, (kobj_class_t)&driver);	\
		console_render.meta = &console_meta;				\
		console_render.cons = cons;					\
		if (cons)							\
			cons->render = &console_render;				\
		kgc_render_register(&driver, 0, 1);				\
		kgc_render_register(&driver, 2, 1);				\
		kgc_render_alloc(0, &console_render);				\
	}

/* Register the render from first to last consoles. */
extern void kgc_render_init(void);
extern render_t kgc_get_render(kgi_u_t devid);
extern render_t kgc_render_alloc(kgi_u_t devid, render_t r);
extern void kgc_render_release(kgi_u_t devid);
extern kgi_error_t kgc_render_register(render_driver_t *render, kgi_u_t display,
				       kgi_u8_t already_allocated);
extern kgi_error_t kgc_render_unregister(render_driver_t *driver);
extern void *kgc_render_meta(render_t r);
extern kgi_console_t *kgc_render_cons(render_t r);

#endif	/* !_kgc_render_h */
