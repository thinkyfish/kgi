/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2002 Nicholas Souchu
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
 * KII keymap manager.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define	KGI_SYS_NEED_IO
#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <sys/kbio.h>
#include <dev/kbd/kbdreg.h>

#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

/*
 * Keymap lookup stuff.
 */

void 
keymap_reset(kii_keymap_t *k)
{
	kii_u_t i;

	if (k && (k != &default_kii_keymap)) {
#		define	LOCAL_FREE(x)				\
			if (k->x != default_kii_keymap.x) {	\
				kgi_kfree(k->x);		\
				KGI_TRACE(0, k->x = NULL);	\
			}

		LOCAL_FREE(fn_buf)
		LOCAL_FREE(fn_str)
		for (i = 0; i < k->keymap_size; i++)
			LOCAL_FREE(keymap[i]);

		LOCAL_FREE(keymap)

		memcpy(k, &default_kii_keymap, sizeof(*k));
#		undef 	LOCAL_FREE
	}
}

kii_s_t 
keymap_set_keysym(kii_keymap_t *k, kii_u_t shift, kii_u_t key,
		kii_unicode_t sym)
{
	kii_unicode_t *newmap;
	kii_u_t i, size;

	KGI_ASSERT(k);

	if ((k == NULL) || (k->keymap_size <= shift) || (key < k->keymin) ||
		(k->keymax < key)) {
		return (KII_EINVAL);
	}
	if (k->keymap[shift] &&
		(k->keymap[shift] != default_kii_keymap.keymap[shift])) {
		k->keymap[shift][key] = sym;
		return( KII_EOK);
	}

	if ((sym == K_VOID) && (k->keymap[shift] == NULL)) 
		return (KII_EOK);

	size = sizeof(kii_keymap_t) + k->fn_buf_size;
	for (i = 0; i < k->keymap_size; i++) {
		if (k->keymap[i]) {
			size += sizeof(kii_unicode_t) * (k->keymax-k->keymin+1);
		}
	}
	size += sizeof(void *) * (k->fn_str_size + k->keymap_size);
	size += sizeof(kii_dead_combination_t) * k->combine_size;
	size += sizeof(kii_unicode_t) * (k->keymax-k->keymin+1);
	if (KII_MAX_KEYMAP_MEMORY < size) 
		return (KII_ENOMEM);

	if (k->keymap == default_kii_keymap.keymap) {
		kii_unicode_t **newarr;

		size = sizeof(k->keymap[0]) * k->keymap_size;
		if ((newarr = kgi_kmalloc(size)) == NULL) {
			return (KII_ENOMEM);
		}
		memcpy(newarr, k->keymap, size);
		k->keymap = newarr;
		KGI_DEBUG(2, "allocated new keymap array (%i slots)", 
			k->keymap_size);
	}
	size = k->keymax - k->keymin + 1;
	if ((newmap = kgi_kmalloc(sizeof(kii_unicode_t) * size)) == NULL) 
		return (KII_ENOMEM);

	KGI_DEBUG(2, "%s keymap %i", 
		default_kii_keymap.keymap[shift] ? "reallocated" : "allocated",
		shift);
	i = size;
	if (default_kii_keymap.keymap[shift]) {
		while (i--) {
			newmap[i] = default_kii_keymap.keymap[shift][i];
		}
	} else {
		while (i--) 
			newmap[i] = K_VOID;
	}
	newmap[key] = sym;
	k->keymap[shift] = newmap;
	KGI_ASSERT(k->keymap[shift] != default_kii_keymap.keymap[shift]);
	return (KII_EOK);
}

kii_s_t keymap_set_default_keysym(kii_u_t shift, kii_u_t key,
	  kii_unicode_t sym)
{
	kii_keymap_t *k = &default_kii_keymap;
	kii_unicode_t *newmap;
	kii_u_t i;

	if ((k->keymap_size <= shift) || (key < k->keymin) ||
		(k->keymax < key)) {
		return (KII_EINVAL);
	}
	if (k->keymap[shift]) {
		k->keymap[shift][key] = sym;
		return (KII_EOK);
	}
	
	if (sym == K_VOID) 
		return (KII_EOK);

	newmap = kgi_kmalloc((k->keymax - k->keymin + 1) 
			* sizeof(kii_unicode_t));
	if (newmap == NULL) 
		return (KII_ENOMEM);
	
	for (i = k->keymax - k->keymin; i; i--) 
		newmap[i] = K_VOID;
	
	newmap[key] = sym;
	k->keymap[shift] = newmap;
	return (KII_EOK);
}

kii_unicode_t 
keymap_get_keysym(kii_keymap_t *k, kii_u_t shift, kii_u_t key)
{
	kii_unicode_t *map, sym;

	if ((key < k->keymin) || (k->keymax < key)) 
		return (K_VOID);

	key -= k->keymin;
	map = (shift < k->keymap_size) ? k->keymap[shift] : NULL;
	sym = map ? map[key] : K_VOID;

	if ((k->keymap[0][key] & K_TYPE_MASK) == K_TYPE_SHIFT) 
		sym = k->keymap[0][key];

	return (sym);
}

kii_unicode_t 
keymap_combine_dead(kii_keymap_t *k, kii_unicode_t diacr, kii_unicode_t base)
{
	kii_u_t i;

	for (i = 0; i < k->combine_size; i++) {
		if ((k->combine[i].diacr == diacr) && 
			(k->combine[i].base == base)) {
			return (k->combine[i].combined);
		}
	}
	switch (K_TYPE(base)) {
	case K_TYPE_SPECIAL: /* Fall thru. */
	case K_TYPE_SHIFT:
		return (K_VOID);
	default:
		return (diacr);
	}
}

const kii_ascii_t *
keymap_get_fnstring(kii_keymap_t *k, kii_u_t key)
{

	return ((key < k->fn_str_size) ? k->fn_str[key] : NULL);
}

kii_s_t 
keymap_set_fnstring(kii_keymap_t *k, kii_u_t key, const kii_ascii_t *s)
{

	KGI_DEBUG(2, "keymap_set_fnstring() not implemented yet!");
	return (KII_EINVAL);
}

struct __kii_keymap_translation_t { kii_unicode_t val1, val2, xor; };

static const struct __kii_keymap_translation_t keymap_range_translation[] = {
	{ 0x0041, 0x005a,  0x0020 }, { 0x0061, 0x007a,  0x0020 },
	{ 0x00c0, 0x00de,  0x0020 }, { 0x00e0, 0x00fe,  0x0020 },
	{ 0x0100, 0x012f,  0x0001 }, { 0x0132, 0x0137,  0x0001 },
	{ 0x014a, 0x0177,  0x0001 }, { 0x0182, 0x0185,  0x0001 },
	{ 0x01a0, 0x01a5,  0x0001 }, { 0x01e0, 0x01ef,  0x0001 },
	{ 0x01fa, 0x01ff,  0x0001 }, { 0x0200, 0x0217,  0x0001 },
	{ 0x0391, 0x039f,  0x0020 }, { 0x03a0, 0x03ab,  0x0060 },
	{ 0x03b1, 0x03bf,  0x0020 }, { 0x03c0, 0x03cb,  0x0060 },
	{ 0x03e2, 0x03ef,  0x0001 }, { 0x0401, 0x040f,  0x0050 },
	{ 0x0410, 0x041f,  0x0020 }, { 0x0420, 0x042f,  0x0060 },
	{ 0x0430, 0x043f,  0x0020 }, { 0x0440, 0x044f,  0x0060 },
	{ 0x0451, 0x045f,  0x0050 }, { 0x0460, 0x0481,  0x0001 },
	{ 0x0490, 0x04bf,  0x0001 }, { 0x04d0, 0x04f9,  0x0001 },
	{ 0x0531, 0x053f,  0x0050 }, { 0x0540, 0x054f,  0x0030 },
	{ 0x0550, 0x0556,  0x00d0 }, { 0x0561, 0x056f,  0x0050 },
	{ 0x0570, 0x057f,  0x0030 }, { 0x0580, 0x0586,  0x00d0 },
	{ 0x1e00, 0x1e59,  0x0001 }, { 0x1ea0, 0x1ef9,  0x0001 },
	{ 0x1f00, 0x1f15,  0x0008 }, { 0x1f18, 0x1f1d,  0x0008 },
	{ 0x1f20, 0x1f45,  0x0008 }, { 0x1f48, 0x1f4d,  0x0008 },
	{ 0x1f68, 0x1f60,  0x0008 }, { 0xff21, 0xff3a,  0x0060 },
	{ 0xff41, 0xff5a,  0x0060 }
};

static const struct __kii_keymap_translation_t keymap_pair_translation[] = {
	{ 0x0139, 0x013a,  0x0003 }, { 0x013b, 0x013c,  0x0007 },
	{ 0x013d, 0x013e,  0x0003 }, { 0x013f, 0x0140,  0x007f },
	{ 0x0141, 0x0142,  0x0003 }, { 0x0143, 0x0144,  0x0007 },
	{ 0x0145, 0x0146,  0x0003 }, { 0x0147, 0x0148,  0x000f },
	{ 0x0178, 0x00ff,  0x0187 }, { 0x0179, 0x017a,  0x0003 },
	{ 0x017b, 0x017c,  0x0007 }, { 0x017d, 0x017e,  0x0003 },
	{ 0x0181, 0x0253,  0x03d2 }, { 0x0186, 0x0254,  0x03d2 },
	{ 0x0187, 0x0188,  0x000f }, { 0x018a, 0x0257,  0x03dd },
	{ 0x018b, 0x018c,  0x0007 }, { 0x018e, 0x0258,  0x03d6 },
	{ 0x018f, 0x0259,  0x03d6 }, { 0x0190, 0x025b,  0x03cb },
	{ 0x0191, 0x0192,  0x0003 }, { 0x0193, 0x0260,  0x03f3 },
	{ 0x0194, 0x0263,  0x03f7 }, { 0x0196, 0x0269,  0x03ff },
	{ 0x0197, 0x0268,  0x03ff }, { 0x0198, 0x0199,  0x0001 },
	{ 0x019c, 0x026f,  0x03f3 }, { 0x019d, 0x0272,  0x03ef },
	{ 0x01a7, 0x01a8,  0x000f }, { 0x01a9, 0x0283,  0x032a },
	{ 0x01ac, 0x01ad,  0x0001 }, { 0x01ae, 0x0288,  0x0326 },
	{ 0x01af, 0x01b0,  0x001f }, { 0x01b1, 0x028a,  0x033b },
	{ 0x01b2, 0x028b,  0x0339 }, { 0x01b3, 0x01b4,  0x0007 },
	{ 0x01b5, 0x01b6,  0x0003 }, { 0x01b7, 0x0292,  0x0325 },
	{ 0x01b8, 0x01b9,  0x0001 }, { 0x01bc, 0x01bd,  0x0001 },
	{ 0x01c4, 0x01c6,  0x0002 }, { 0x01c7, 0x01c9,  0x000e },
	{ 0x01ca, 0x01cc,  0x0006 }, { 0x01cd, 0x01ce,  0x0003 },
	{ 0x01cf, 0x01d0,  0x001f }, { 0x01d1, 0x01d2,  0x0003 },
	{ 0x01d3, 0x01d4,  0x0007 }, { 0x01d5, 0x01d6,  0x0003 },
	{ 0x01d7, 0x01d8,  0x000f }, { 0x01d9, 0x01da,  0x0003 },
	{ 0x01db, 0x01dc,  0x0007 }, { 0x01de, 0x01df,  0x0001 },
	{ 0x01f1, 0x01f3,  0x0002 }, { 0x01f4, 0x01f5,  0x0001 },
	{ 0x0386, 0x03ac,  0x002a }, { 0x0388, 0x03ad,  0x0025 },
	{ 0x0389, 0x03ae,  0x0027 }, { 0x038a, 0x03af,  0x0025 },
	{ 0x038c, 0x03cc,  0x0040 }, { 0x038e, 0x03cd,  0x0043 },
	{ 0x038f, 0x03ce,  0x0041 }, { 0x04c1, 0x04c2,  0x0003 },
	{ 0x04c3, 0x04c4,  0x0007 }, { 0x04c7, 0x04c8,  0x000f },
	{ 0x04cb, 0x04cc,  0x0007 }, { 0x1f59, 0x1f51,  0x0008 },
	{ 0x1f5b, 0x1f53,  0x0008 }, { 0x1f5d, 0x1f55,  0x0008 },
	{ 0x1f5f, 0x1f57,  0x0008 }, { 0x1fba, 0x1f70,  0x00ca },
	{ 0x1fbb, 0x1f71,  0x00ca }, { 0x1fc8, 0x1f72,  0x00ba },
	{ 0x1fc9, 0x1f73,  0x00ba }, { 0x1fca, 0x1f74,  0x00be },
	{ 0x1fcb, 0x1f75,  0x00be }, { 0x1fda, 0x1f76,  0x00ac },
	{ 0x1fdb, 0x1f77,  0x00ac }, { 0x1fea, 0x1f7a,  0x0090 },
	{ 0x1feb, 0x1f7b,  0x0090 }, { 0x1fec, 0x1fe5,  0x0009 },
	{ 0x1ff8, 0x1f78,  0x0080 }, { 0x1ff9, 0x1f79,  0x0080 },
	{ 0x1ffa, 0x1f7c,  0x0086 }, { 0x1ffb, 0x1f7d,  0x0086 },
	{ 0x2112, 0x2113,  0x0001 }, { 0x2130, 0x212f,  0x001f }
};

kii_unicode_t 
keymap_toggled_case(kii_unicode_t sym)
{
	kii_u_t i, size;

	size = sizeof(keymap_pair_translation) /
		sizeof(keymap_pair_translation[0]);

	for (i = 0; i < size; i++) {
		if ((keymap_pair_translation[i].val1 == sym) || 
			(keymap_pair_translation[i].val2 == sym)) {
			return (sym ^ keymap_pair_translation[i].xor);
		}
	}

	size = sizeof(keymap_range_translation) /
		   sizeof(keymap_range_translation[0]);

	for (i = 0; i < size; i++) {
		if ((keymap_range_translation[i].val1 <= sym) &&
			(sym <= keymap_range_translation[i].val2)) {
			return (sym ^ keymap_range_translation[i].xor);
		}
	}

	if ((0x24b6 <= sym) && (sym <= 0x24cf))
		return (sym + 26);
	
	if ((0x24d0 <= sym) && (sym <= 0x24e9)) 
		return (sym - 26);
	
	return (sym);
}
