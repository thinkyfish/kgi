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
 * From FreeBSD: src/sys/dev/fb/splash.c,v 1.14 2004/05/30 20:08:31 phk Exp
 *
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/linker.h>

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_backgnd.h>

/* decoder candidates */
#define MAX_NR_DECODERS 4
static backgnd_decoder_t *decoder_set[MAX_NR_DECODERS];

static backgnd_decoder_t *backgnd_decoders[KGI_MAX_NR_DEVICES];
static backgnd_callback_t backgnd_callbacks[KGI_MAX_NR_DEVICES];
static void *backgnd_args[KGI_MAX_NR_DEVICES];

static void
kgc_backgnd_init(void) {

	bzero(decoder_set, sizeof(decoder_set));
	bzero(backgnd_decoders, sizeof(backgnd_decoders));
	bzero(backgnd_callbacks, sizeof(backgnd_callbacks));
	bzero(backgnd_args, sizeof(backgnd_args));
}

SYSINIT(backgnd, SI_SUB_KGI, SI_ORDER_ANY, kgc_backgnd_init, NULL);

static int
backgnd_find_data(backgnd_decoder_t *decoder)
{
        caddr_t image_module;
	caddr_t p;

	if (decoder->data_type == NULL)
		return 0;
	image_module = preload_search_by_type(decoder->data_type);
	if (image_module == NULL)
		return ENOENT;
	p = preload_search_info(image_module, MODINFO_ADDR);
	if (p == NULL)
		return ENOENT;
	decoder->data = *(void **)p;
	p = preload_search_info(image_module, MODINFO_SIZE);
	if (p == NULL)
		return ENOENT;
	decoder->data_size = *(kgi_u32_t *)p;
	KRN_BOOT("backgnd: image@%p, size:%lu\n",
		 (void *)decoder->data, (long)decoder->data_size);
	return 0;
}

static int
backgnd_test(backgnd_decoder_t *decoder, kgi_u_t devid)
{
	render_t r;

	if (!(r = kgc_get_render(devid)))
		return EINVAL;

	decoder->data = NULL;
	decoder->data_size = 0;

	/* Let the decoder provide its own defaults if backgnd
	 * data not found.
	 */
	backgnd_find_data(decoder);

	if (*decoder->init && (*decoder->init)(r)) {
		decoder->data = NULL;
		decoder->data_size = 0;
		return ENODEV;	/* XXX */
	}
	if (bootverbose)
		printf("backgnd: image decoder found: %s\n", decoder->name);
	return 0;
}

static void
backgnd_new(backgnd_decoder_t *decoder, kgi_u_t devid)
{
	render_t r;

	if (!(r = kgc_get_render(devid)))
		return;

	backgnd_decoders[devid] = decoder;
	if (backgnd_callbacks[devid] != NULL)
		(*backgnd_callbacks[devid])(r, BACKGND_INIT, backgnd_args[devid]);
}

int
backgnd_register(backgnd_decoder_t *decoder)
{
	int i, error = ENOMEM;
	kgi_u_t devid;

	/* register the decoder for later use */
	for (i = 0; i < MAX_NR_DECODERS; ++i) {
		if (decoder_set[i] == NULL) {
			decoder_set[i] = decoder;
			error = 0;
			break;
		}
	}

	/* Parse the devids to find any render that would
	 * have not decoder.
	 */
	for (devid=0; KGI_VALID_DEVICE_ID(devid); devid++) {
		if (backgnd_callbacks[devid] && !backgnd_decoders[devid]) {
			if (backgnd_test(decoder, devid) == 0) {
				backgnd_new(decoder, devid);
			}
		}
	}

	return error;
}

int
backgnd_unregister(backgnd_decoder_t *decoder)
{
	int error;
	kgi_u_t i, j;
	render_t r;

	if (!decoder)
		return 0;

	for(j=0; j<MAX_NR_DECODERS; j++) {
		if (decoder_set[j] == decoder)
			break;
	}	

	if (j >= MAX_NR_DECODERS)
		return EINVAL;

	for(i=0; KGI_VALID_DEVICE_ID(i); i++) {
		if (backgnd_decoders[i] == decoder) {
			if (!(r = kgc_get_render(i)))
				continue;
			if ((error = backgnd_term(i)) != 0) {
				KRN_ERROR("Can't terminate backgnd %d (%d)", i, error);
			}
		}
	}
	decoder_set[i] = NULL;

	return 0;
}

int
backgnd_init(kgi_u_t devid, backgnd_callback_t callback, void *arg)
{
	int i, error = ENODEV;

	if (!KGI_VALID_DEVICE_ID(devid))
		return EINVAL;

	backgnd_callbacks[devid] = callback;
	backgnd_args[devid] = arg;

	backgnd_decoders[devid] = NULL;

	/* Scan the decoders to find one */
	for (i = 0; i < MAX_NR_DECODERS; ++i) {
		if (decoder_set[i] == NULL)
			continue;
		if (backgnd_test(decoder_set[i], devid) == 0) {
			backgnd_new(decoder_set[i], devid);
			error = 0;
			break;
		}
	}
	return error;
}

int
backgnd_term(kgi_u_t devid)
{
	int error = 0;
	render_t r;

	if (!KGI_VALID_DEVICE_ID(devid) || !(r = kgc_get_render(devid)))
		return EINVAL;

	if (backgnd_decoders[devid] != NULL) {
		if (backgnd_callbacks[devid]!= NULL)
			error = (*backgnd_callbacks[devid])(r, BACKGND_TERM,
							backgnd_args[devid]);
		if (error == 0 && backgnd_decoders[devid]->term)
			error = (*backgnd_decoders[devid]->term)(r);
	}

	/* XXX Free decoder and callback entries anyway */
	backgnd_decoders[devid] = NULL;
	backgnd_callbacks[devid] = NULL;
	backgnd_args[devid] = NULL;
	
	return error;
}

int
backgnd_draw(kgi_u_t devid, unsigned char *mem, kgi_u16_t *pal)
{
	render_t r;

	if (!KGI_VALID_DEVICE_ID(devid) || !(r = kgc_get_render(devid)))
		return EINVAL;

	if (backgnd_decoders[devid] != NULL)
		return (*backgnd_decoders[devid]->draw)(r, mem, pal);

	return ENODEV;
}
