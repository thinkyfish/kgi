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
 * KII keysym definitions.
 */

#ifndef	_KII_KIISYMS_H_
#define	_KII_KIISYMS_H_

#define	KII_MAX_NR_SHIFT	16

#ifdef	KII_NEED_MODIFIER_KEYSYMS

enum __kii_ledtypes {
	KBD_LED_SCROLL_LOCK	= 0x00000001,
	KBD_LED_CAPS_LOCK	= 0x00000002,
	KBD_LED_NUM_LOCK	= 0x00000004
};

enum __kii_modifiers {
	KII_MK_SHIFT,
	KII_MK_ALTGR,
	KII_MK_CTRL,
	KII_MK_ALT,
	KII_MK_SHIFTL,
	KII_MK_SHIFTR,
	KII_MK_CTRLL,
	KII_MK_CTRLR
};

enum __kii_modifier_masks {
#define	KII_MASK(x)	KII_MM_##x = (1 << KII_MK_##x)
	KII_MASK(SHIFT),
	KII_MASK(ALTGR),
	KII_MASK(CTRL),
	KII_MASK(ALT),
	KII_MASK(SHIFTL),
	KII_MASK(SHIFTR),
	KII_MASK(CTRLL),
	KII_MASK(CTRLR),
#undef	KII_MASK
	KII_MM_ALL = (1 << KII_MAX_NR_SHIFT) - 1
};

enum __kii_keytypes {
	K_TYPE_LATIN = 0x0000,	/* Must be (and is) zero! */
	K_TYPE_FUNCTION	= 0xF100,
		K_FIRST_FUNCTION = K_TYPE_FUNCTION,
		K_F1 = K_FIRST_FUNCTION,
		K_F2,
		K_F3,
		K_F4,
		K_F5,
		K_F6,
		K_F7,
		K_F8,
		K_F9,
		K_F10,
		K_F11,
		K_F12,
		K_F13,
		K_F14,
		K_F15,
		K_F16,
		K_F17,
		K_F18,
		K_F19,
		K_F20,
		K_FIND,
		K_INSERT,
		K_REMOVE,
		K_SELECT,
		K_PGUP,
		K_PGDN,
		K_MACRO,
		K_HELP,
		K_DO,
		K_PAUSE,
		K_F21,
		K_F22,
		K_F23,
		K_F24,
		K_F25,
		K_F26,
		K_F27,
		K_F28,
		K_F29,
		K_F30,
		K_F31,
		K_F32,
		K_F33,
		K_F34,
		K_F35,
		K_F36,
		K_F37,
		K_F38,
		K_F39,
		K_F40,
		K_F41,
		K_F42,
		K_F43,
		K_F44,
		K_F45,
		K_F46,
		K_F47,
		K_F48,
		K_F49,
		K_F50,
		K_F51,
		K_F52,
		K_F53,
		K_F54,
		K_F55,
		K_F56,
		K_F57,
		K_F58,
		K_F59,
		K_F60,
		K_F61,
		K_F62,
		K_F63,
		K_F64,
		K_F65,
		K_F66,
		K_F67,
		K_F68,
		K_F69,
		K_F70,
		K_F71,
		K_F72,
		K_F73,
		K_F74,
		K_F75,
		K_F76,
		K_F77,
		K_F78,
		K_F79,
		K_F80,
		K_F81,
		K_F82,
		K_F83,
		K_F84,
		K_F85,
		K_F86,
		K_F87,
		K_F88,
		K_F89,
		K_F90,
		K_F91,
		K_F92,
		K_F93,
		K_F94,
		K_F95,
		K_F96,
		K_F97,
		K_F98,
		K_F99,
		K_F100,
		K_F101,
		K_F102,
		K_F103,
		K_F104,
		K_F105,
		K_F106,
		K_F107,
		K_F108,
		K_F109,
		K_F110,
		K_F111,
		K_F112,
		K_F113,
		K_F114,
		K_F115,
		K_F116,
		K_F117,
		K_F118,
		K_F119,
		K_F120,
		K_F121,
		K_F122,
		K_F123,
		K_F124,
		K_F125,
		K_F126,
		K_F127,
		K_F128,
		K_F129,
		K_F130,
		K_F131,
		K_F132,
		K_F133,
		K_F134,
		K_F135,
		K_F136,
		K_F137,
		K_F138,
		K_F139,
		K_F140,
		K_F141,
		K_F142,
		K_F143,
		K_F144,
		K_F145,
		K_F146,
		K_F147,
		K_F148,
		K_F149,
		K_F150,
		K_F151,
		K_F152,
		K_F153,
		K_F154,
		K_F155,
		K_F156,
		K_F157,
		K_F158,
		K_F159,
		K_F160,
		K_F161,
		K_F162,
		K_F163,
		K_F164,
		K_F165,
		K_F166,
		K_F167,
		K_F168,
		K_F169,
		K_F170,
		K_F171,
		K_F172,
		K_F173,
		K_F174,
		K_F175,
		K_F176,
		K_F177,
		K_F178,
		K_F179,
		K_F180,
		K_F181,
		K_F182,
		K_F183,
		K_F184,
		K_F185,
		K_F186,
		K_F187,
		K_F188,
		K_F189,
		K_F190,
		K_F191,
		K_F192,
		K_F193,
		K_F194,
		K_F195,
		K_F196,
		K_F197,
		K_F198,
		K_F199,
		K_F200,
		K_F201,
		K_F202,
		K_F203,
		K_F204,
		K_F205,
		K_F206,
		K_F207,
		K_F208,
		K_F209,
		K_F210,
		K_F211,
		K_F212,
		K_F213,
		K_F214,
		K_F215,
		K_F216,
		K_F217,
		K_F218,
		K_F219,
		K_F220,
		K_F221,
		K_F222,
		K_F223,
		K_F224,
		K_F225,
		K_F226,
		K_F227,
		K_F228,
		K_F229,
		K_F230,
		K_F231,
		K_F232,
		K_F233,
		K_F234,
		K_F235,
		K_F236,
		K_F237,
		K_F238,
		K_F239,
		K_F240,
		K_F241,
		K_F242,
		K_F243,
		K_F244,
		K_F245,
		K_UNDO,
		K_LAST_FUNCTION,
		K_PRIOR = K_PGUP,
		K_NEXT  = K_PGDN,
	K_TYPE_SPECIAL	= 0x0000F200,
		K_FIRST_SPECIAL = K_TYPE_SPECIAL,
		K_VOID = K_FIRST_SPECIAL,
		K_HOLE = K_VOID,
		K_ENTER,
		K_SH_REGS,
		K_SH_MEM,
		K_SH_STAT,
		K_BREAK,
		K_CONS,
		K_CAPS,
		K_NUM,
		K_HOLD,
		K_SCROLLFORW,
		K_SCROLLBACK,
		K_BOOT,
		K_CAPSON,
		K_COMPOSE,
		K_SAK,
		K_DECRCONSOLE,
		K_INCRCONSOLE,
		K_SPAWNCONSOLE,
		K_BARENUMLOCK,
		K_TOGGLESCREEN,
		K_SYSTEM_REQUEST,
		/* Here under, added for FreeBSD */
		K_HALT,
		K_POWERDOWN,
		K_SUSPEND,
		K_STANDBY,
		K_PANIC,
		K_LAST_SPECIAL,
		K_NOSUCHMAP = K_TYPE_SPECIAL + 127,
	K_TYPE_NUMPAD = 0x0000F300,
		K_FIRST_NUMPAD = K_TYPE_NUMPAD,
		K_P0 = K_FIRST_NUMPAD,
		K_P1,
		K_P2,
		K_P3,
		K_P4,
		K_P5,
		K_P6,
		K_P7,
		K_P8,
		K_P9,
		K_PPLUS,
		K_PMINUS,
		K_PSTAR,
		K_PSLASH,
		K_PENTER,
		K_PCOMMA,
		K_PDOT,
		K_PPLUSMINUS,
		K_PARENL,
		K_PARENR,
		K_LAST_NUMPAD,
		K_PASTERISK = K_PSTAR,
	K_TYPE_DEAD = 0x0000F400,
		K_FIRST_DEAD = K_TYPE_DEAD,
		K_DGRAVE = K_FIRST_DEAD,
		K_DACUTE,
		K_DCIRCM,
		K_DTILDE,
		K_DDIERE,
		K_DCEDIL,
		K_LAST_DEAD,
	K_TYPE_CONSOLE 	= 0x0000F500,
		K_FIRST_CONSOLE = K_TYPE_CONSOLE,
		K_LAST_CONSOLE = K_FIRST_CONSOLE + CONFIG_KGII_MAX_NR_CONSOLES,
	K_TYPE_CURSOR 	= 0x0000F600,
		K_FIRST_CURSOR = K_TYPE_CURSOR,
		K_DOWN	= K_FIRST_CURSOR,
		K_LEFT,
		K_RIGHT,
		K_UP,
		/* Here under, added for FreeBSD. */
		K_HOME,
		K_END,
		K_LAST_CURSOR,
	K_TYPE_SHIFT	= 0x0000F700,
		K_FIRST_SHIFT = K_TYPE_SHIFT,
		K_FIRST_NORMAL = K_FIRST_SHIFT,
		K_NORMAL_SHIFT = K_FIRST_NORMAL,
		K_NORMAL_ALTGR,
		K_NORMAL_CTRL,
		K_NORMAL_ALT,
		K_NORMAL_SHIFTL,
		K_NORMAL_SHIFTR,
		K_NORMAL_CTRLL,	
		K_NORMAL_CTRLR,
		K_LAST_NORMAL  = K_FIRST_NORMAL + KII_MAX_NR_SHIFT,
		K_FIRST_LOCKED = K_LAST_NORMAL,
		K_LOCKED_SHIFT = K_FIRST_LOCKED,
		K_LOCKED_CTRL,
		K_LOCKED_ALT,
		K_LOCKED_ALTGR,
		K_LOCKED_SHIFTL,
		K_LOCKED_SHIFTR,
		K_LOCKED_CTRLL,
		K_LOCKED_CTRLR,
		K_LAST_LOCKED  = K_FIRST_LOCKED + KII_MAX_NR_SHIFT,
		K_FIRST_STICKY = K_LAST_LOCKED,
		K_STICKY_SHIFT = K_FIRST_STICKY,
		K_STICKY_CTRL,
		K_STICKY_ALT,
		K_STICKY_ALTGR,
		K_STICKY_SHIFTL,
		K_STICKY_SHIFTR,
		K_STICKY_CTRLL,
		K_STICKY_CTRLR,
		K_LAST_STICKY  = K_FIRST_STICKY + KII_MAX_NR_SHIFT,
		K_LAST_SHIFT = K_LAST_STICKY,
	K_TYPE_META	= 0x0000F800,
		K_LAST_META,
	K_TYPE_ASCII	= 0x0000F900,
		K_FIRST_ASCII = K_TYPE_ASCII,
		K_LAST_ASCII,
	K_TYPE_MASK	= 0x0000FF00,
	K_VALUE_MASK	= 0x000000FF
};
#define	K_SYM(type, val)	((type) | (val))
#define	K_TYPE(x)	((x) & K_TYPE_MASK)
#define	K_VALUE(x)	((x) & K_VALUE_MASK)
#endif

/* Symbols of type special. */
#define	KS_VOID		"Void"
#define	KS_HOLE		"Hole"
#define	KS_ENTER	"Enter"
#define	KS_SH_REGS	"SH-Regs"
#define	KS_SH_MEM	"SH-Mem"
#define	KS_SH_STAT	"SH-Stat"
#define	KS_BREAK	"Break"
#define	KS_CONS		"Cons"
#define	KS_CAPS		"Caps"
#define	KS_NUM		"Num"
#define	KS_HOLD		"Hold"
#define	KS_SCROLLFORW	"ScrollForw"
#define	KS_SCROLLBACK	"ScrollBack"
#define	KS_BOOT		"Boot"
#define	KS_CAPSON	"CapsON"
#define	KS_COMPOSE	"Compose"
#define	KS_SAK		"SAK"
#define	KS_DECRCONSOLE	"DecrCons"
#define	KS_INCRCONSOLE	"IncrCons"
#define	KS_SPAWNCONSOLE	"SpawnCons"
#define	KS_BARENUMLOCK	"BareNumLock"
#define	KS_TOGGLESCREEN	"ToggleScreen"
#define	KS_SYSTEM_REQUEST "SysRequest"
#define KS_HALT		"Halt"
#define KS_POWERDOWN	"PowerDown"
#define KS_SUSPEND	"Suspend"
#define KS_PANIC	"Panic"
#define KS_HOME		"Home"
#define KS_END		"End"

/* Symbols of type function. */
#define KS_F1		"F1"
#define KS_F2		"F2"
#define KS_F3		"F3"
#define KS_F4		"F4"
#define KS_F5		"F5"
#define KS_F6		"F6"
#define KS_F7		"F7"
#define KS_F8		"F8"
#define KS_F9		"F9"
#define KS_F10		"F10"
#define KS_F11		"F11"
#define KS_F12		"F12"
#define KS_F13		"F13"
#define KS_F14		"F14"
#define KS_F15		"F15"
#define KS_F16		"F16"
#define KS_F17		"F17"
#define KS_F18		"F18"
#define KS_F19		"F19"
#define KS_F20		"F20"
#define KS_FIND		"Find"
#define KS_INSERT	"Insert"
#define KS_REMOVE	"Remove"
#define KS_SELECT	"Select"
#define KS_PGUP		"PgUp"
#define KS_PGDN		"PgDown"
#define KS_MACRO	"Macro"
#define KS_HELP		"Help"
#define KS_DO		"Do"
#define KS_PAUSE	"Pause"

/* Symbols of type NumPad. */
#define KS_P0		"P0"
#define KS_P1		"P1"
#define KS_P2		"P2"
#define KS_P3		"P3"
#define KS_P4		"P4"
#define KS_P5		"P5"
#define KS_P6		"P6"
#define KS_P7		"P7"
#define KS_P8		"P8"
#define KS_P9		"P9"
#define KS_PPLUS	"P+"
#define KS_PMINUS	"P-"
#define KS_PSTAR	"P*"
#define KS_PSLASH	"P/"
#define KS_PENTER	"PRet"
#define KS_PCOMMA	"PComma"
#define KS_PDOT		"P."
#define KS_PPLUSMINUS	"P+/-"
#define KS_PARENL	"P("
#define KS_PARENR	"P)"

/* Symbols of type shift. */
#define KS_NORMAL_SHIFT		"Shift"
#define KS_NORMAL_CTRL		"Ctrl"
#define KS_NORMAL_ALT		"Alt"
#define KS_NORMAL_ALTGR		"AltGr"
#define KS_NORMAL_SHIFTL	"ShiftL"
#define KS_NORMAL_SHIFTR	"ShiftR"
#define KS_NORMAL_CTRLL		"CtrlL"
#define KS_NORMAL_CTRLR		"CtrlR"

#define KS_LOCKED_SHIFT		"L-Shift"
#define KS_LOCKED_CTRL		"L-Ctrl"
#define KS_LOCKED_ALT		"L-Alt"
#define KS_LOCKED_ALTGR		"L-AltGr"
#define KS_LOCKED_SHIFTL	"L-ShiftL"
#define KS_LOCKED_SHIFTR	"L-ShiftR"
#define KS_LOCKED_CTRLL		"L-CtrlL"
#define KS_LOCKED_CTRLR		"L-CtrlR"

#define KS_STICKY_SHIFT		"S-Shift"
#define KS_STICKY_CTRL		"S-Ctrl"
#define KS_STICKY_ALT		"S-Alt"
#define KS_STICKY_ALTGR		"S-AltGr"
#define KS_STICKY_SHIFTL	"S-ShiftL"
#define KS_STICKY_SHIFTR	"S-ShiftR"
#define KS_STICKY_CTRLL		"S-CtrlL"
#define KS_STICKY_CTRLR		"S-CtrlR"

extern char *keysyms_pretty_print(int sym);

#endif /* _KII_KIISYMS_H_ */
