/* ----------------------------------------------------------------------------
**	Linux console emulation (compatibility module)
** ----------------------------------------------------------------------------
**	partial Copyright (C)	1998-2000	Steffen Seeger
**
**	This code was derived from linux-2.1/drivers/char/vt.c, thus additional
**	copyrights apply. Please see there for full history. However, most 
**	has been rewritten from scratch.
**
**	This file is distributed under the terms and conditions of the 
**	GNU General Public License. Please see the file COPYRIGHT.GPL included
**	with this distribution of the terms and conditions of this license.
** ----------------------------------------------------------------------------
**
**	$Log: vt.c,v $
**	Revision 1.3  2002/10/15 00:39:22  aldot
**	- remove pre-2.4
**	- add linux-2.4.19 support
**	- include cleanup
**	
**	Revision 1.2  2002/07/27 01:12:45  aldot
**	- add linux-2.5 support
**	
**	Revision 1.1.1.1  2000/04/18 08:50:54  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:54  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <linux/config.h>
#include <linux/version.h>
#include "kgi/config.h"
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/interrupt.h>

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include <asm/uaccess.h>

#include <linux/kgii.h>
#include <kgi/system.h>
#ifndef CONFIG_INPUT
#define KII_NEED_MODIFIER_KEYSYMS
#endif
#include <kii/kii.h>
#define __KGI_NEED_MALLOC
#include <kgi/alloc.h>
#include "console.h"
#include "psaux.h"

#define	EOK	0

#if LINUX_VERSION_CODE < 0x020500
#define KEYBOARD_BH CONSOLE_BH
#endif

extern kii_device_t *kiidevice[KII_MAX_NR_DEVICES];

unsigned int vt_dont_switch = 0;

/* Console (vt and kd) routines, as defined by USL SVR4 manual, and by
 * experimentation and study of X386 SYSV handling.
 *
 * One point of difference: SYSV vt's are /dev/vtX, which X >= 0, and
 * /dev/console is a separate ttyp. Under Linux, /dev/tty0 is /dev/console,
 * and the vc start at /dev/ttyX, X >= 1. We maintain that here, so we will
 * always treat our set of vt as numbered 1..MAX_NR_CONSOLES (corresponding to
 * ttys 0..MAX_NR_CONSOLES-1). Explicitly naming VT 0 is illegal, but using
 * /dev/tty0 (fg_console) as a target is legal, since an implicit aliasing
 * to the current console is done by the main ioctl code.
 */
#if 0
/*
 * This function is called when the size of the physical screen has been
 * changed.  If either the row or col argument is nonzero, set the appropriate
 * entry in each winsize structure for all the virtual consoles, then
 * send SIGWINCH to all processes with a virtual console as controlling
 * tty.
 */

static int
kd_size_changed(int row, int col)
{
	struct task_struct *p;
	int i;

	if ( !row && !col ) return 0;

	for ( i = 0 ; i < MAX_NR_CONSOLES ; i++ ) {

		if ( console_driver.table[i] ) {

			if (row) {
				console_driver.table[i]->winsize.ws_row = row;
			}
			if (col) {
				console_driver.table[i]->winsize.ws_col = col;
			}
		}
	}

	read_lock(&tasklist_lock);
	for_each_task(p) {

		if (p->tty && MAJOR(p->tty->device) == TTY_MAJOR &&
			MINOR(p->tty->device) <= MAX_NR_CONSOLES &&
			MINOR(p->tty->device)) {

			send_sig(SIGWINCH, p, 1);
		}
	}
	read_unlock(&tasklist_lock);

	return 0;
}
#endif


/*	Sometimes we want to wait until a particular VT has been activated. We
**	do it in a very simple manner. Everybody waits on a single queue and
**	get woken up at once. Those that are satisfied go on with their
**	business, while those not ready go back to sleep. Seems overkill
**	to add a wait to each vt just for this - usually this does nothing!
*/

DECLARE_WAIT_QUEUE_HEAD(vt_activate_queue);

/*	Sleeps until a vt is activated, or the task is interrupted. Returns
**	0 if activation, -EINTR if interrupted.
*/
int vt_waitactive(kii_device_t *dev, int vt)
{
	kii_focus_t *f = kiidev_focus(dev->id);
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);

	add_wait_queue(&vt_activate_queue, &wait);
	for (;;) {

		current->state = TASK_INTERRUPTIBLE;
		if (KII_VALID_CONSOLE_ID(f->curr_console) &&
			(vt == f->console_map[f->curr_console])) {

			break;
		}
		if (signal_pending(current)) {

			retval = -EINTR;
			break;
		}
		schedule();
	}
	remove_wait_queue(&vt_activate_queue, &wait);
	current->state = TASK_RUNNING;
	return retval;
}

static inline void vt_wake_waitactive(void)
{
	wake_up(&vt_activate_queue);
}

static void vt_set_leds(kgi_console_t *cons)
{
	kii_u_t leds = cons->kd.led;

	cons->kii.flags &= ~KII_DF_LED_FLAGS;
	if (leds & K_SCROLLLOCK) {
		cons->kii.flags |= KII_DF_SCROLL_LOCK;
	}
	if (leds & K_CAPSLOCK) {
		cons->kii.flags |= KII_DF_CAPS_SHIFT;
	}
	if (leds & K_NUMLOCK) {
		cons->kii.flags |= KII_DF_NUM_LOCK;
	}

	kiidev_sync(&cons->kii, KII_SYNC_LED_FLAGS);
}

int vt_handle_kii_event(kgi_console_t *cons, kii_event_t *ev)
{
	kii_u32_t mask = 1 << ev->any.type;
	struct tty_struct *tty = cons->kii.tty;

	if (mask & KII_EM_POINTER) {

#ifdef CONFIG_INPUT
		/* mouse_handle_kii_event(ev);  ??? */
#else
		psaux_handle_kii_event(ev);
#endif
	}

	if (mask & ~(KII_EM_KEY | KII_EM_RAW_DATA)) {

		return 0;
	}

	if (cons->kd.kbmode == VC_RAW) {

		if (mask & KII_EM_RAW_DATA) {

			kii_u_t i, cnt = ev->raw.size - sizeof(kii_any_event_t);
			KRN_ASSERT(cnt < (sizeof(kii_raw_event_t) - 
				sizeof(kii_any_event_t)));

			for (i = 0; i < cnt; i++) {

				tty_insert_flip_char(tty, ev->raw.data[i], 0);
			}
			tty_schedule_flip(tty);
		}
		return 1;
	}

	if (cons->kd.kbmode == VC_MEDIUMRAW) {

		if ((mask & KII_EM_KEY) && (ev->key.code < 128)) {

			if (ev->key.type == KII_EM_KEY_RELEASE) {

				tty_insert_flip_char(tty, ev->key.code | 0x80,
					0);
			} else {
				KRN_ASSERT((1<<ev->key.type) & 
					(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT));
				tty_insert_flip_char(tty, ev->key.code, 0);
			}
			tty_schedule_flip(tty);
		}
		return 1;
	}
	return 0;
}


#define	IOCTL(x)	KRN_ERROR("%s: " x , current->comm)


/*	We handle the console-specific ioctl's here.  We allow the
**	capability to modify any console, not just the fg_console. 
*/
int vt_ioctl(struct tty_struct *tty, struct file * file,
	     unsigned int cmd, unsigned long arg)
{
	union
	{
		unsigned char	uchar;
		unsigned int 	uint;
		struct kbentry	kbe;
		struct kbsentry	kse;
		struct kbdiacrs	kde;
		struct vt_stat	vts;
		struct vt_mode	vtm;
		struct { unsigned int ticks; unsigned int count; } tone;

	} kern, *user = (void *) arg;

	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;
	kii_focus_t *f = kiidev_focus(cons->kii.id);

	unsigned int i, err, size;
	const char *src;
	char *dst;

	if (cons == NULL) {

		return -ENODEV;
	}
	switch (cmd) {

	case KDGKBTYPE:
		return put_user(KB_101, &user->uchar);

	case KDGETMODE:
		return put_user(cons->kd.mode, &user->uint);

	case KDGKBMODE:
		return put_user(cons->kd.kbmode, &user->uint);

	case KDGKBMETA:
		return put_user(cons->kd.kbmeta, &user->uint); 

	case KDSKBMETA:
		if (! ((arg == K_METABIT) || (arg == K_ESCPREFIX))) {

			return -EINVAL;
		}
		cons->kd.kbmeta = arg;
		return EOK;

	case KDGKBENT:
		if (copy_from_user(&kern.kbe, &user->kbe, sizeof(kern.kbe))) {

			return -EFAULT;
		}
		kern.kbe.kb_value = keymap_get_keysym(&f->kmap,
			kern.kbe.kb_table, kern.kbe.kb_index);
		return put_user(kern.kbe.kb_value, &user->kbe.kb_value);

	case KDGKBSENT:
		if (copy_from_user(&kern.kse, &user->kse, sizeof(kern.kse))) {

			return -EFAULT;
		}
		kern.kse.kb_string[sizeof(kern.kse.kb_string) - 1] = '\0';

		size = sizeof(kern.kse.kb_string) - 1;
		dst = user->kse.kb_string;
		src = keymap_get_fnstring(&f->kmap, kern.kse.kb_func);
		if (src) {

			while (*src && size) {

				__put_user(*src, dst);
				src++, dst++, size--;
			}
		}
		__put_user('\0', dst);
		return (src && *src) ? -EOVERFLOW : EOK;

	case KDGKBDIACR:
	{
		struct kbdiacr *kbdc = user->kde.kbdiacr;
		kii_dead_combination_t *combine = f->kmap.combine;

		for (i = 0; i < f->kmap.combine_size; i++) {

			if (	(err = put_user(combine[i].diacr,
					&kbdc[i].diacr)) ||
				(err = put_user(combine[i].base,
					&kbdc[i].base)) ||
				(err = put_user(combine[i].combined,
					&kbdc[i].result))) {

				return err;
			}
		}
		return put_user(f->kmap.combine_size, &user->kde.kb_cnt);
	}

	case KDGKBLED:
		return put_user(cons->kd.kbled, (char*)arg);

	case KDGETLED:
		return put_user(cons->kd.led, (char*) arg);
	}

	/*	For all the following ioctl's, you need to be owner or
	**	superuser to have permission to perform that ioctl.
	*/
	if ((current->tty != tty) && !capable(CAP_SYS_ADMIN)) {

		return -EPERM;
	}

	switch (cmd) {

	case KIOCSOUND:
		kii_make_sound(f, arg, 0);
		return EOK;

	case KDMKTONE:
		/*	Generate the tone for the appropriate number of ticks.
		**	If the time is zero, turn off sound ourselves.
		*/
		kern.tone.ticks = (arg >> 16) & 0xffff;
		kern.tone.count = kern.tone.ticks ? (arg & 0xffff) : 0;
		kii_make_sound(f, kern.tone.count, kern.tone.ticks);
		return EOK;

	case KDSETMODE:
		switch (arg) {
		case KD_TEXT:
		case KD_TEXT0:
		case KD_TEXT1:
			arg = KD_TEXT;
		case KD_GRAPHICS:
			cons->kd.mode = arg;
			return EOK;
		default:
			return -EINVAL;
		}

	case KDSKBMODE:
		switch (arg) {
		case K_RAW:
			cons->kd.kbmode = VC_RAW;
			break;
		case K_MEDIUMRAW:
			cons->kd.kbmode = VC_MEDIUMRAW;
			break;
		case K_XLATE:
			cons->kd.kbmode = VC_XLATE;
			break;
		case K_UNICODE:
			cons->kd.kbmode = VC_UNICODE;
			break;
		default:
			return -EINVAL;
		}
		if (tty->ldisc.flush_buffer) {

			tty->ldisc.flush_buffer(tty);
		}
		return EOK;

	case KDGETKEYCODE:
	case KDSETKEYCODE:
		/*	KII allows more than one keyboard per focus and
		**	has to provide more flexible means to set the
		**	device-internal translation tables. Thus these
		**	are gone. Maybe we should return EOK?
		*/
		return -ENOIOCTLCMD;

	case KDSKBENT:
		/* ... no special permissions required! */
		if (copy_from_user(&kern.kbe, &user->kbe, sizeof(kern.kbe))) {

			return -EFAULT;
		}
		if ((kern.kbe.kb_index == 0) &&
			(kern.kbe.kb_value == K_NOSUCHMAP)) {

			kii_u_t shift = kern.kbe.kb_table;
			if (shift < f->kmap.keymap_size) {

				kfree(f->kmap.keymap[shift]);
				f->kmap.keymap[shift] = NULL;
				KRN_DEBUG(1, "disallocated keymap %i", shift);
			}
			return 0;
		}
#warning	check SAK, etc.
		kern.kbe.kb_value ^= 0xF000;
		switch (kern.kbe.kb_value & 0xFF00) {
		case 0xF000:
		case 0xFB00:
			kern.kbe.kb_value &= 0x00FF;
		}
		return keymap_set_keysym(&f->kmap, kern.kbe.kb_table,
			kern.kbe.kb_index, kern.kbe.kb_value);

	case KDSKBSENT:
		if (copy_from_user(&kern.kse, &user->kse, sizeof(kern.kse))) {

			return -EFAULT;
		}
		kern.kse.kb_string[sizeof(kern.kse.kb_string) - 1] = '\0';
		return keymap_set_fnstring(&f->kmap, kern.kse.kb_func,
			kern.kse.kb_string);


	case KDSKBDIACR:
	{
		unsigned int cnt;
		struct kbdiacr *kbdc = user->kde.kbdiacr;
		kii_dead_combination_t *combine = f->kmap.combine;

		if ((err = verify_area(VERIFY_READ, user, sizeof(user->kde)))) {

			return err;
		}
		__get_user(cnt, &user->kde.kb_cnt);
		if (cnt >= f->kmap.combine_size) {

			return -EINVAL;
		}
		for (i = 0; i < cnt; i++) {

			__get_user(combine[i].diacr, &kbdc[i].diacr);
			__get_user(combine[i].base,  &kbdc[i].base);
			__get_user(combine[i].combined, &kbdc[i].result);
		}
		while (i < f->kmap.combine_size) {

			combine[i].diacr = combine[i].base =
			combine[i].combined = K_VOID;
			i++;
		}
		return EOK;
	}

	/*	set flags usually shown in the leds.
	**	Don't use, may go away without warning!
	*/
	case KDSKBLED:
		if (arg & ~0x77) {

			return -EINVAL;
		}
		cons->kd.kbled = arg;
		vt_set_leds(cons);
		return EOK;

	/*	set lights, (not flags)
	*/
	case KDSETLED:
		cons->kd.led = arg;
		vt_set_leds(cons);
		return EOK;

	/*	A process can indicate its willingness to accept signals
	**	generated by pressing an appropriate key combination.
	**	Thus, one can have a daemon that e.g. spawns a new console
	**	upon a keypress and then changes to it.
	**	Probably init should be changed to do this (and have a
	**	field ks (`keyboard signal') in inittab describing the
	**	desired action), so that the number of background daemons
	**	does not increase.
	*/
	case KDSIGACCEPT:
		if (arg < 1 || arg > _NSIG || arg == SIGKILL) {

			return -EINVAL;
		}
		cons->kd.spawnpid = current->pid;
		cons->kd.spawnsig = arg;
		return EOK;

	case VT_SETMODE:
		if (copy_from_user(&kern.vtm, &user->vtm, sizeof(kern.vtm))) {

			return -EFAULT;
		}
		if (kern.vtm.mode != VT_AUTO && kern.vtm.mode != VT_PROCESS) {

			return -EINVAL;
		}
		cons->vt_mode = kern.vtm;
		cons->vt_mode.frsig = 0;	/* ignored */
		cons->vt_pid = current->pid;
		cons->vt_ack = VT_ACK_IDLE;
		return EOK;

	case VT_GETMODE:
		/* ... no special permissions required! */
		return  copy_to_user((void*) arg, &(cons->vt_mode),
			sizeof(cons->vt_mode)) ? -EFAULT : EOK; 

	/*	Returns global vt state. Note that VT 0 is always open, since
	**	it's an alias for the current VT, and people can't use it here.
	**	We cannot return state for more than 16 VTs, since v_state
	**	is short.
	*/
	case VT_GETSTATE:
	{
		unsigned short state, mask;

		/* ... no special permissions required! */
		if ((err = verify_area(VERIFY_WRITE, user, 
			sizeof(user->vts)))) {

			return err;
		}
		__put_user(f->console_map[f->curr_console] + 1,
			&user->vts.v_active);
		state = 1;	/* /dev/tty0 is always open ??? */
		for (i = 0, mask = 2; i < KII_MAX_NR_CONSOLES && mask;
			++i, mask <<= 1) {

			if (KII_VALID_DEVICE_ID(f->console_map[i]) &&
				kiidevice[f->console_map[i]]) {

				state |= mask;
			}
		}
		return __put_user(state, &user->vts.v_state);
	}

	/*
	** Returns the first available (non-opened) console.
	*/
	case VT_OPENQRY:
		/* ... no special permissions required! */
		for (i = 0; i < KII_MAX_NR_CONSOLES; ++i) {

			if (KII_VALID_DEVICE_ID(f->console_map[i]) &&
				!kiidevice[f->console_map[i]]) {

				KRN_DEBUG(2, "OPENQRY: console %i, device %i",
					i, f->console_map[i]);
				return put_user(f->console_map[i]+1, (int *) arg);
			}
		}
		KRN_DEBUG(2, "OPENQRY: didn't find free console.");
		return put_user(-1, (int *) arg);		 

	/*	ioctl(fd, VT_ACTIVATE, num) will cause us to switch to 
	**	vt # num, with num >= 1 (switches to vt 0, our console,
	**	are not allowed, just to preserve sanity).
	*/
	case VT_ACTIVATE:
		if ((arg == 0) || (KII_MAX_NR_DEVICES < arg)) {

			return -ENXIO;
		}
		/* ???	the KII bottom half handler checks if want_console
		** ???	is a valid device and forgets the switch.
		** ??? 	The original code tried to allocate a new console, so
		** ???	we should at least try to report an error...
		*/
		KRN_DEBUG(0, "activating switch to %i", arg - 1);
		{
			int console = 0;
			f->want_console = KII_INVALID_CONSOLE;
			while (console < KII_MAX_NR_CONSOLES) {

				if (f->console_map[console] == (arg-1)) {

					f->want_console = console;
					break;
				}
				console++;
			}
		}
		f->flags |= KII_FF_PROCESS_BH;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		mark_bh(KEYBOARD_BH);
#else
		tasklet_schedule(&keyboard_tasklet);
#endif
		return EOK;

	/*	wait until the specified VT has been activated
	*/
	case VT_WAITACTIVE:
		if (arg == 0 || KII_MAX_NR_DEVICES < arg) {

			return -ENXIO;
		}
		{
			int i;

			for (i = 0; i < KII_MAX_NR_CONSOLES; i++) {

				if (f->console_map[i] == (arg-1)) {

					break;
				}
			}
			if (i == KII_MAX_NR_CONSOLES) {

				return -EINVAL;
			}
		}

		return vt_waitactive(&cons->kii, arg-1);

	/*	If a vt is under process control, the kernel will not 
	**	switch to it immediately, but postpone the operation 
	**	until the process calls this ioctl, allowing the switch
	**	to complete.
	**	According to the X sources this is the behavior:
	**	0:	pending switch-from not OK
	**	1:	pending switch-from OK
	**	2:	completed switch-to OK
	*/
	case VT_RELDISP:
		if (cons->vt_mode.mode != VT_PROCESS) {

			KRN_ERROR("VT_RELDISP: console not in PROCESS mode");
			return -EINVAL;
		}
		switch (arg) {

		case 0:
			if (cons->vt_ack == VT_ACK_PENDING_FROM) {

				cons->vt_ack = VT_ACK_IDLE;
				f->want_console = KII_INVALID_CONSOLE;
				KRN_ERROR("pending from disallowed");
				return EOK;
			}
			break;

		case 1:
			if (cons->vt_ack == VT_ACK_PENDING_FROM) {

				KRN_DEBUG(3, "pending from acknowledged");
				cons->vt_ack = VT_ACK_DONE;
				f->flags |= KII_FF_PROCESS_BH;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
				mark_bh(KEYBOARD_BH);
#else
				tasklet_schedule(&keyboard_tasklet);
#endif
				return EOK;
			}
			KRN_ERROR("invalid pending-from-ack");
			break;

		case 2:
			if (cons->vt_ack == VT_ACK_PENDING_TO) {

				KRN_DEBUG(3, "pending to acknowledged");
				cons->vt_ack = VT_ACK_IDLE;
				return EOK;
			}
			KRN_ERROR("invalid pendig-to-ack");
			break;
		}
		return -EINVAL;

#if 0
	case VT_RESIZE:
		IOCTL("VT_RESIZE");
	{
		struct vt_sizes *vtsizes = (struct vt_sizes *) arg;
		ushort ll,cc;
		i = verify_area(VERIFY_READ, (void *)vtsizes, sizeof(struct vt_sizes));
		if (i)
			return i;
		__get_user(ll, &vtsizes->v_rows);
		__get_user(cc, &vtsizes->v_cols);
		i = vc_resize(ll, cc, 0, CONFIG_KGII_MAX_NR_CONSOLES);
		return i ? i : 	kd_size_changed(ll, cc);
	}

	case VT_RESIZEX:
		IOCTL("VT_RESIZEX");
	{
		struct vt_consize *vtconsize = (struct vt_consize *) arg;
		ushort ll,cc,vlin,clin,vcol,ccol;
		i = verify_area(VERIFY_READ, (void *)vtconsize, sizeof(struct vt_consize));
		if (i)
			return i;
		__get_user(ll, &vtconsize->v_rows);
		__get_user(cc, &vtconsize->v_cols);
		__get_user(vlin, &vtconsize->v_vlin);
		__get_user(clin, &vtconsize->v_clin);
		__get_user(vcol, &vtconsize->v_vcol);
		__get_user(ccol, &vtconsize->v_ccol);
		vlin = vlin ? vlin : video_scan_lines;
		if ( clin )
		  {
		    if ( ll )
		      {
			if ( ll != vlin/clin )
			  return -EINVAL; /* Parameters don't add up */
		      }
		    else 
		      ll = vlin/clin;
		  }
		if ( vcol && ccol )
		  {
		    if ( cc )
		      {
			if ( cc != vcol/ccol )
			  return -EINVAL;
		      }
		    else
		      cc = vcol/ccol;
		  }

		if ( clin > 32 )
		  return -EINVAL;
		    
		if ( vlin )
		  video_scan_lines = vlin;
		if ( clin )
		  video_font_height = clin;
		
		i = vc_resize(ll, cc);
		if (i)
			return i;

		kd_size_changed(ll, cc);
		return 0;
	}
#endif
	case VT_LOCKSWITCH:
	case VT_UNLOCKSWITCH:
		if (!capable(CAP_SYS_ADMIN)) {

			return -EPERM;
		}
		vt_dont_switch = (cmd == VT_LOCKSWITCH);
		return EOK;

	default:
		return -ENOIOCTLCMD;
	}
}

void vt_reset_vc(kgi_console_t *cons)
{
	cons->kii.event_mask |= KII_EM_RAW_DATA;

	cons->kd.kbmode = VC_XLATE;
	cons->vc_mode  = KD_TEXT;

	cons->vt_mode.mode = VT_AUTO;
	cons->vt_mode.waitv = 0;
	cons->vt_mode.relsig = 0;
	cons->vt_mode.acqsig = 0;
	cons->vt_mode.frsig = 0;
	cons->vt_ack = VT_ACK_IDLE;

	cons->vt_pid = -1;

/* !!!	console_reset_palette(cons); */
}

/*
**	Performs the back end of a vt switch
*/
void console_map_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = (kgi_console_t *) dev->priv.priv_ptr;

	switch (cons->vt_mode.mode) {
	/*	WARNING !!!     Make sure you understand the flow    !!!
	**	WARNING !!! through this code before messing with it !!!
	*/

	case VT_PROCESS:
		/*	in process mode we have to tell a process when switching
		*/
		switch (cons->vt_ack) {

		case VT_ACK_IDLE:
			if (!kill_proc(cons->vt_pid, cons->vt_mode.acqsig, 1)) {

				/*	It worked. The process has to send a
				**	VT_RELDISP ioctl to complete the switch.
				*/
				KRN_DEBUG(3, "expecting pending-to-ack");
				cons->vt_ack = VT_ACK_PENDING_TO;
				return;
			}
			/*	The controlling process has died, or is 
			**	intentionally not responding so we revert
			**	back to normal operation (KD_TEXT,VT_AUTO).
			**	This doesn't reset the display hardware, but
			**	may allow a 'blind' login.
			*/
			KRN_NOTICE("process died? resetting console");
			vt_reset_vc(cons);
			break;

		default:
			KRN_INTERNAL_ERROR;
			return;
		}
		/*	fall through
		*/

	case VT_AUTO:
		vt_wake_waitactive();
		console_do_kgi_map(cons);
		return;

	default:
		KRN_INTERNAL_ERROR;
		return;
	}
}

/*	Performs the front-end of a vt switch
*/
int console_unmap_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = (kgi_console_t *) dev->priv.priv_ptr;

	switch (cons->vt_mode.mode) {
	/*	WARNING!!!     Make sure you understand the flow    !!!
	**	WARNING!!! through this code before messing with it !!!
	*/

	case VT_PROCESS:
		/*	In process mode we have to handshake with a process
		**	before switching. Essentially we wait for it to tell
		**	us when	it is ready (via VT_RELDISP ioctl). 
		*/
		switch (cons->vt_ack) {

		case VT_ACK_PENDING_TO:
			/*	The process is still completing a 'to' (map)
			**	switch. We can't do more than ignore the switch.
			*/
			KRN_ERROR("unmap requested but map not complete");
			return -EINVAL;

		case VT_ACK_IDLE:
			if (!kill_proc(cons->vt_pid, cons->vt_mode.relsig, 1)) {

				/*	It worked. The process has to send a
				**	VT_RELDISP ioctl to complete the switch.
				*/
				KRN_DEBUG(3, "expecting pending-from-ack");
				cons->vt_ack = VT_ACK_PENDING_FROM;
				return -EAGAIN;
			}
			/*	fall through
			*/
		case VT_ACK_PENDING_FROM:
			/*	The controlling process has died, or is 
			**	intentionally not responding so we revert
			**	back to normal operation (KD_TEXT,VT_AUTO).
			**	This doesn't reset the display hardware, but
			**	may allow a 'blind login'.
			*/
			KRN_NOTICE("process died? resetting console");
			vt_reset_vc(cons);
		case VT_ACK_DONE:
			KRN_DEBUG(3, "console switch done.");
			cons->vt_ack = VT_ACK_IDLE;
			break;

		default:
			KRN_INTERNAL_ERROR;
			return -EINVAL;
		}
		/*	fall through
		*/

	case VT_AUTO:
		/*	Ignore all switches in KD_GRAPHICS mode
		*/
		if (cons->vc_mode != KD_GRAPHICS) {

			console_do_kgi_unmap(cons);
			return EOK;
		}
		return -EINVAL;

	default:
		KRN_INTERNAL_ERROR;
		return -EINVAL;
	}
}

void vt_init(void)
{
#ifndef CONFIG_INPUT
	psaux_init();
#endif
}
