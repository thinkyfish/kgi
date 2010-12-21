/*-
 * Copyright (c) 1996-2000 Steffen Seeger
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
 * XTERM console parser state
 */

#ifndef	_KGI_XTERM_H_
#define	_KGI_XTERM_H_

#define	XTERM_MAX_NPAR	16

enum kgi_console_mode_xterm_e {
	KGI_CM_XT_DISPLAY_CTRL = KGI_CM_LAST, /* display control characters*/
	KGI_CM_XT_TOGGLE_META,		/* set meta bit 0x80		*/
	KGI_CM_XT_UTF8,			/* do utf8 decoding		*/
	KGI_CM_XT_APPLIC_KEY,		/* keypad in application mode	*/
	KGI_CM_XT_CURSOR_KEY,		/* DECCKM - cursor key mode	*/
	KGI_CM_XT_COLUMN132,		/* 132 column mode		*/
	KGI_CM_XT_X10_MOUSE,		/* x10 compatible mouse report	*/
	KGI_CM_XT_TEXTRONIX_MODE,	/* enter textronix mode		*/
	KGI_CM_XT_ALLOW_132MODE,	/* allow switch to 132 columns	*/
	KGI_CM_XT_CURSES_FIX,		/* allow hack for curses	*/
	KGI_CM_XT_MARGIN_BELL,		/* margin bell			*/
	KGI_CM_XT_LOGGING,		/* do logging			*/
	KGI_CM_XT_REPORT_MOUSE,		/* report mouse			*/
	KGI_CM_XT_TRACK_MOUSE,		/* report & hilite track mouse	*/
	KGI_CM_XT_ANSI,			/* DECANM - ANSI mode		*/
	KGI_CM_XT_NEWLINE,		/* ENTER sends CR/CRLF		*/
	KGI_CM_XT_INSERT,		/* ANSI insert mode		*/
	KGI_CM_XT_META_ESC,		/* META chars with ESC-prefix	*/
	KGI_CM_XT_LAST
};

enum kgi_console_flags_xterm_e {
	KGI_CF_XT_QUESTION	= 0x40000000,	/* ESC?[ sequence?	*/
	KGI_CF_XT_NUM_LOCK	= 0x20000000,	/* num_lock key active	*/
	KGI_CF_XT_CAPS_LOCK	= 0x10000000,	/* CAPS-shift, locked	*/
	KGI_CF_XT_CAPS_STICKY	= 0x08000000,	/* CAPS-shift, sticky	*/
	KGI_CF_XT_ALL		= 0x78000000
};


enum xterm_states_e {
	ESnormal,	ESesc,		ESsquare,	ESgetpars,
	ESgotpars,	ESfunckey, 	EShash,		ESset,
	ESpercent,	ESignore
};

typedef struct {
	kgi_console_t	cons;
	kgi_u8_t	state;
	kgi_u8_t	charset;	
	kgi_u8_t	s_charset;
	kgi_u8_t	utf_count;

	struct {
		kgi_u_t pitch, duration;
	} bell;

	kgi_u_t		npar;
	unsigned long	par[XTERM_MAX_NPAR];
	kgi_isochar_t	utf_char;
	kgi_unicode_t	*translate;
	kgi_s_t		s_x, s_y;	/* saved cursor position	*/
	kgi_s_t		p_x, p_y;	/* saved pointer position	*/
	kgi_u8_t	s_g[4];
	kgi_u8_t	g[4];
	kgi_u8_t	colors[8];
	kgi_u8_t	s_colors[8];
	kgi_u_t		s_attrfl;
	unsigned long	s_mode;	/* saved modes			*/
} kgi_console_xterm_t;

#define	LAT1_MAP	0
#define	GRAF_MAP	1
#define	IBMPC_MAP	2
#define	USER_MAP	3

extern void xterm_do_reset(kgi_console_t *cons, int do_reset);
extern void xterm_handle_kii_event(kii_device_t *dev, kii_event_t *ev);
extern int xterm_do_write(kgi_console_t *cons, const char *buf, int count);

#endif	/* _KGI_XTERM_H_ */
