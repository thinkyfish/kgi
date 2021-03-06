/* ----------------------------------------------------------------------------
**	XTERM console parser state
** ----------------------------------------------------------------------------
**	Copyright (C)	1996-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: xterm.h,v $
*/
#ifndef	_KGI_XTERM_H
#define	_KGI_XTERM_H

#define	XTERM_MAX_NPAR	16
#define	XTERM_MAX_COLUMNS	160
#define	XTERM_TAB_STOPS	XTERM_MAX_COLUMNS/(8*sizeof(unsigned long))


enum kgi_console_mode_xterm_e
{
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

enum kgi_console_flags_xterm_e 
{
	KGI_CF_XT_QUESTION	= 0x40000000,	/* ESC?[ sequence?	*/
	KGI_CF_XT_NUM_LOCK	= 0x20000000,	/* num_lock key active	*/
	KGI_CF_XT_CAPS_LOCK	= 0x10000000,	/* CAPS-shift, locked	*/
	KGI_CF_XT_CAPS_STICKY	= 0x08000000,	/* CAPS-shift, sticky	*/

	KGI_CF_XT_ALL		= 0x78000000
};


enum xterm_states_e
{
	ESnormal,	ESesc,		ESsquare,	ESgetpars,
	ESgotpars,	ESfunckey, 	EShash,		ESset,
	ESpercent,	ESignore
};

typedef struct
{
	kgi_console_t	cons;

	kgi_u8_t	state;
	kgi_u8_t	charset;	
	kgi_u8_t	s_charset;
	kgi_u8_t	utf_count;

	kgi_u_t		npar;
	unsigned long	par[XTERM_MAX_NPAR];

	unsigned long	tab_stop[XTERM_TAB_STOPS];

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
extern int xterm_do_write(kgi_console_t *cons, int from_user,
		const char *buf, int count);

#endif	/* #ifdef _KGI_XTERM_H*/
