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
 * KGI console implementation specific definitions
 */

#ifndef _KGC_RENDER_H_
#define	_KGC_RENDER_H_

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
#define DECLARE_RENDER(name,driver,meta_type)				       \
	static meta_type console_meta;					       \
	static struct render console_render;				       \
	static struct kobj_ops render_ops;				       \
	void name##_configure(kgi_console_t *cons) {			       \
		kobj_class_compile_static((kobj_class_t)&driver, &render_ops); \
		kobj_init((kobj_t)&console_render, (kobj_class_t)&driver);     \
		console_render.meta = &console_meta;			       \
		console_render.cons = cons;				       \
		if (cons)						       \
			cons->render = &console_render;			       \
		kgc_render_register(&driver, 0, 1);			       \
		kgc_render_register(&driver, 2, 1);			       \
		kgc_render_alloc(0, &console_render);			       \
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

#endif	/* _KGC_RENDER_H_ */
