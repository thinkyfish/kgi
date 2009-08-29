/*-
 * Copyright (c) 1999 Kazutaka YOKOTA <yokota@zodiac.mech.utsunomiya-u.ac.jp>
 * Copyright (c) 2003 Nicholas Souchu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * From FreeBSD: src/sys/dev/fb/splashreg.h,v 1.6 2004/07/15 08:26:01 phk Exp
 *
 * $FreeBSD$
 *
 */

#ifndef _kgc_backgnd_h
#define	_kgc_backgnd_h

#define BACKGND_IMAGE	"splash_image_data"

struct render_t;

struct backgnd_decoder {
	char		*name;
	int		(*init)(render_t r);
	int		(*term)(render_t r);
	int		(*draw)(render_t r, unsigned char *mem, kgi_u16_t *pal);
	char		*data_type;
	void		*data;
	size_t		data_size;
};

typedef struct backgnd_decoder	backgnd_decoder_t;

typedef int (*backgnd_callback_t)(render_t, int, void *);

#define BACKGND_DECODER(name, sw)				\
	static int name##_modevent(module_t mod, int type, void *data) \
	{							\
		switch ((modeventtype_t)type) {			\
		case MOD_LOAD:					\
			return backgnd_register(&sw);		\
		case MOD_UNLOAD:				\
			return backgnd_unregister(&sw);		\
		default:					\
			break;					\
		}						\
		return 0;					\
	}							\
	static moduledata_t name##_mod = {			\
		#name, 						\
		name##_modevent,				\
		NULL						\
	};							\
	DECLARE_MODULE(name, name##_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);

/* entry point for the splash image decoder */
extern int	backgnd_register(backgnd_decoder_t *decoder);
extern int	backgnd_unregister(backgnd_decoder_t *decoder);

/* entry points for the render */
extern int	backgnd_init(kgi_u_t, backgnd_callback_t, void *arg);
extern int	backgnd_term(kgi_u_t devid);
extern int	backgnd_draw(kgi_u_t devid, unsigned char *mem, kgi_u16_t *pal);

/* event types for the callback function */
#define BACKGND_INIT	0
#define BACKGND_TERM	1

#endif /* !_kgc_backgnd_h */
