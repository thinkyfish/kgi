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
 * KII keysyms pretty-printer.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#define	KGI_SYS_NEED_IO
#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kii/kiisyms.h>

char *modifier_str[] = {
	"Shift", "Ctrl", "Alt", "AltGr", "ShiftL", "ShiftR",
	"CtrlL", "CtrlR", NULL
};

char *type_function_str[] = {
	KS_F1,
	KS_F2,
	KS_F3,
	KS_F4,
	KS_F5,
	KS_F6,
	KS_F7,
	KS_F8,
	KS_F9,
	KS_F10,
	KS_F11,
	KS_F12,
	KS_F13,
	KS_F14,
	KS_F15,
	KS_F16,
	KS_F17,
	KS_F18,
	KS_F19,
	KS_F20,
	KS_FIND,
	KS_INSERT,
	KS_REMOVE,
	KS_SELECT,
	KS_PGUP,
	KS_PGDN,
	KS_MACRO,
	KS_HELP,
	KS_DO,
	KS_PAUSE,
	NULL
};

char *type_numpad_str[] = {
	KS_P0,
	KS_P1,
	KS_P2,
	KS_P3,
	KS_P4,
	KS_P5,
	KS_P6,
	KS_P7,
	KS_P8,
	KS_P9,
	KS_PPLUS,
	KS_PMINUS,
	KS_PSTAR,
	KS_PSLASH,
	KS_PENTER,
	KS_PCOMMA,
	KS_PDOT,
	KS_PPLUSMINUS,
	KS_PARENL,
	KS_PARENR,
	NULL
};

char *type_special_str[] = {
	KS_VOID,
	KS_ENTER,
	KS_SH_REGS,
	KS_SH_MEM,
	KS_SH_STAT,
	KS_BREAK,
	KS_CONS,
	KS_CAPS,
	KS_NUM,
	KS_HOLD,
	KS_SCROLLFORW,
	KS_SCROLLBACK,
	KS_BOOT,
	KS_CAPSON,
	KS_COMPOSE,
	KS_SAK,
	KS_DECRCONSOLE,
	KS_INCRCONSOLE,
	KS_SPAWNCONSOLE,
	KS_BARENUMLOCK,
	KS_TOGGLESCREEN,
	KS_SYSTEM_REQUEST,
	NULL
};

char *type_shift_normal_str[] = {
	KS_NORMAL_SHIFT,
	KS_NORMAL_CTRL,
	KS_NORMAL_ALT,
	KS_NORMAL_ALTGR,
	KS_NORMAL_SHIFTL,
	KS_NORMAL_SHIFTR,
	KS_NORMAL_CTRLL,	
	KS_NORMAL_CTRLR,
	NULL
};

char *type_shift_locked_str[] = {
	KS_LOCKED_SHIFT,
	KS_LOCKED_CTRL,
	KS_LOCKED_ALT,
	KS_LOCKED_ALTGR,
	KS_LOCKED_SHIFTL,
	KS_LOCKED_SHIFTR,
	KS_LOCKED_CTRLL,
	KS_LOCKED_CTRLR,
	NULL
};

char *type_shift_sticky_str[] = {
	KS_STICKY_SHIFT,
	KS_STICKY_CTRL,
	KS_STICKY_ALT,
	KS_STICKY_ALTGR,
	KS_STICKY_SHIFTL,
	KS_STICKY_SHIFTR,
	KS_STICKY_CTRLL,
	KS_STICKY_CTRLR,
	NULL
};

char *
keysyms_pretty_print(int sym)
{
	sym &= 0xFFFF;

	if ((sym >= K_FIRST_FUNCTION) && (sym < K_LAST_FUNCTION))
		return (type_function_str[sym - K_FIRST_FUNCTION]);
	else if ((sym >= K_FIRST_SPECIAL) && (sym < K_LAST_SPECIAL))
		return (type_special_str[sym - K_FIRST_SPECIAL]);		
	else if ((sym >= K_FIRST_NUMPAD) && (sym < K_LAST_NUMPAD))
		return (type_numpad_str[sym - K_FIRST_NUMPAD]);
	else if ((sym >= K_FIRST_NORMAL) && (sym < K_LAST_NORMAL))
		return (type_shift_normal_str[sym - K_FIRST_NORMAL]);
	else if ((sym >= K_FIRST_LOCKED) && (sym < K_LAST_LOCKED))
		return (type_shift_locked_str[sym - K_FIRST_LOCKED]);
	else if ((sym >= K_FIRST_STICKY) && (sym < K_LAST_STICKY))
		return (type_shift_sticky_str[sym - K_FIRST_STICKY]);
	else
		return (NULL);
};
