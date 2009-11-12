/*-
 * Copyright (c) 1998-2000 Steffen Seeger
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

#ifndef _kgc_console_h
#define	_kgc_console_h

/* extern struct tasklet_struct keyboard_tasklet, console_tasklet; */

typedef struct {
	unsigned char	kbmeta;
	unsigned char	mode;
	unsigned char	kbmode;
#define	VC_RAW		0
#define	VC_XLATE	1
#define	VC_MEDIUMRAW	2
#define	VC_UNICODE	3
	unsigned char	kbled;
	unsigned char	led;
} kgi_console_kd_t;

typedef enum {
	KGI_CM_SMOOTH_SCROLL,	/* smooth(slow) scroll		*/
	KGI_CM_REVERSE_VIDEO,	/* black on white		*/
	KGI_CM_ORIGIN,		/* cursor origin mode		*/
	KGI_CM_AUTO_WRAP,	/* automatic line wrap		*/
	KGI_CM_AUTO_REPEAT,	/* repeat chars automatically	*/
	KGI_CM_REVERSE_WRAP,	/* allow reverse wrapping	*/
	KGI_CM_ALT_SCREEN,	/* use alternate screen		*/
	KGI_CM_SHOW_CURSOR,	/* show/hide cursor mark	*/
	KGI_CM_SHOW_POINTER,	/* show/hide pointer mark	*/
	KGI_CM_LAST /* NOTE: there *must* not be more than 32 modes! */
} kgi_console_mode_t;

#define	CONSOLE_CLEAR_MODE(cons, m)	((cons)->mode &= ~(1 << (m)))
#define	CONSOLE_SET_MODE(cons, m)	((cons)->mode |=  (1 << (m)))
#define	CONSOLE_MODE(cons, m)		((cons)->mode &   (1 << (m)))

typedef enum {
#define	KGI_CA_COLOR(fg,bg)	(((bg) & 0xFF) | (((fg) & 0xFF) << 8))
	KGI_CA_FG_COLOR	= 0x0000FF00,
	KGI_CA_BG_COLOR = 0x000000FF,
	KGI_CA_NORMAL	= 0x00000000,
	KGI_CA_HALF	= 0x00010000,
	KGI_CA_BRIGHT	= 0x00020000,
	KGI_CA_INTENSITY= 0x00030000,
	KGI_CA_UNDERLINE= 0x00040000,
	KGI_CA_BOLD	= 0x00080000,
	KGI_CA_ITALIC	= 0x00100000,
	KGI_CA_REVERSE	= 0x00200000,
	KGI_CA_BLINK	= 0x00400000,
	KGI_CA_ALPHA	= 0x00800000
} kgi_console_attributes_t;

typedef enum {
	KGI_CF_NEED_WRAP	= 0x00000001,
	KGI_CF_SCROLLED		= 0x00000002,
	KGI_CF_NO_HARDSCROLL	= 0x00000004,
	KGI_CF_SPLITLINE	= 0x00000008,
	KGI_CF_CURSOR_SHOWN	= 0x00000010,
	KGI_CF_CURSOR_TO_SHOW	= 0x00000020,
	KGI_CF_POINTER_SHOWN	= 0x00000040,
	KGI_CF_POINTER_TO_SHOW	= 0x00000080
} kgi_console_flags_t;

typedef struct kgi_console_s kgi_console_t;

#define MAX_IO_BUF_SIZE			32

struct kgi_console_s {
	kgi_u_t	refcnt;
	kii_device_t	kii;
	int (*DoWrite)(kgi_console_t *, const char *, int);
	void *meta_console;
	/* input handling variables (mostly for cn) */
#define EMPTY 0x0
#define FULL 0x1
#define HAS_DATA 0x2
	int status;
	int head, tail, size;
	int buffer[MAX_IO_BUF_SIZE];
	kgi_console_flags_t flags;
	kgi_console_mode_t  mode;
	/*
	 * scroller and render are defined void* to avoid
	 * .h dependencies
	 */
	void *parser;
	void *scroller;
	void *render;
};

enum kgi_console_color_e {
	KGI_CC_BLACK = 0,
	KGI_CC_BLUE,
	KGI_CC_GREEN,
	KGI_CC_CYAN,
	KGI_CC_RED,
	KGI_CC_MAGENTA,
	KGI_CC_BROWN,
	KGI_CC_LIGHTGRAY,
	KGI_CC_DARKGRAY,
	KGI_CC_LIGHTBLUE,
	KGI_CC_LIGHTGREEN,
	KGI_CC_LIGHTCYAN,
	KGI_CC_LIGHTRED,
	KGI_CC_LIGHTMAGENTA,
	KGI_CC_YELLOW,
	KGI_CC_WHITE
};

/* Pixel colour depths. */
#define EIGHTBIT_COLOUR		8
#define HIGHCOLOUR_15BIT	15
#define HIGHCOLOUR_16BIT	16
#define TRUECOLOUR_24BIT	24
#define DEEPCOLOUR_32BIT	48

/* ASCII control code definitions */
#define	ASCII_NUL	0	/* null (end of string) */
#define	ASCII_SOH	1	/* start of heading	*/
#define	ASCII_STX	2	/* start of text	*/
#define	ASCII_ETX	3	/* end of text		*/
#define	ASCII_EOT	4	/* end of transmission	*/
#define	ASCII_ENQ	5	/* enquiry		*/
#define	ASCII_ACK	6	/* acknowledge		*/
#define	ASCII_BEL	7	/* bell			*/
#define	ASCII_BS	8	/* backspace		*/
#define	ASCII_HT	9	/* horizontal tab	*/
#define	ASCII_TAB	9	/* horizontal tab	*/

#define	ASCII_LF	10	/* line feed	        */
#define	ASCII_VT	11	/* vertical tab		*/
#define	ASCII_FF	12	/* form feed		*/
#define	ASCII_CR	13	/* carriage return	*/
#define	ASCII_SO	14	/* shift out		*/
#define	ASCII_SI	15	/* shift in		*/
#define	ASCII_DLE	16	/* data line escape	*/
#define	ASCII_DC1	17	/* dev-ctrl 1 (X-ON)	*/
#define	ASCII_DC2	18	/* dev-ctrl 2       	*/
#define	ASCII_DC3	19	/* dev-ctrl 3 (X-OFF)	*/

#define	ASCII_DC4	20	/* dev-ctrl 4       	*/
#define	ASCII_NAK	21	/* negative acknowledge	*/
#define	ASCII_SYN	22	/* synchronous idle	*/
#define	ASCII_ETB	23	/* end of transmit block*/
#define	ASCII_CAN	24	/* cancel		*/
#define	ASCII_EM	25	/* end of medium	*/
#define	ASCII_SUB	26	/* substitute		*/
#define	ASCII_ESC	27	/* escape		*/
#define	ASCII_FS	28	/* file separator	*/
#define	ASCII_GS	29	/* group separator	*/

#define	ASCII_RS	30	/* record separator	*/
#define	ASCII_US	31	/* unit separator	*/

extern const kgi_u16_t			default_color_text16_ilut[16 * 3];
extern const kgi_rgb_color_t		default_ptr_color[3];
extern const kgi_u8_t			default_ptr_64x64[1024];

extern void kgc_map_kgi(kgi_device_t *dev);
extern kgi_s_t kgc_unmap_kgi(kgi_device_t *dev);

extern void kgc_map_kii(kii_device_t *dev);
extern kii_s_t kgc_unmap_kii(kii_device_t *dev);

#endif	/* !_kgc_console_h */
