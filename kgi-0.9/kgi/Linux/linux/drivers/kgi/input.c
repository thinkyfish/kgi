/* ---------------------------------------------------------------------------
**	input-linux --> KII event translator
** ---------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Brian S. Julin
**
**	This file is distributed under the terms and conditions of the
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** -------------------------------------------------------------------------
**
**	$Log: input.c,v $
**	Revision 1.1.1.1  2000/04/18 08:50:51  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#include <kgi/maintainers.h>
#define	MAINTAINER	Brian_S_Julin

#define DRIVER_NAME	"input-linux driver"
#define DRIVER_REV	"$Revision: 1.1.1.1 $"
#define	DEBUG_LEVEL	1

#include <linux/config.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("MIT/X");
MODULE_AUTHOR("Maintainer: " MAINTAINER);
MODULE_DESCRIPTION(DRIVER_NAME);

/* TODO: use slab.h come 2.4.15 */
#include <linux/malloc.h>

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/random.h>
#include <linux/input.h>
#include <linux/kgii.h>

#include <kgi/debug.h>

struct pt_regs *kbd_pt_regs;

/* for raw keycode emulation... */
static unsigned char raw_emu_e0s[] = 
{
	0x1c, 0x1d, 0x35, 0x2a, 0x38, 0x39, 0x47, 0x48,
	0x49, 0x4b, 0x4d, 0x4f, 0x50, 0x51, 0x52, 0x53
};

void kii_kbd_event (struct input_handle *handle, unsigned int type, 
		    unsigned int code, int value)
{
	kii_event_t event;
	kii_u_t keycode;
	kii_input_t *kbd;

        if (type != EV_KEY) {

		 return;
	}
	keycode = code & 0xff;
	add_keyboard_randomness(keycode | ((!value) << 7));

	kbd = (kii_input_t *)(handle->private);
	event.any.focus  = kbd->focus;
	event.any.device = kbd->id;
	event.any.time   = jiffies;

	if (kbd->report & KII_EM_RAW_DATA) {
	  
		event.raw.type	  = KII_EV_RAW_DATA;
		event.raw.size	  = sizeof(kii_any_event_t) + 1;
	  
		/* following adapted from Vojtech's code
		*/
	  
		if (keycode >= 96 && keycode <= 111) {

			event.raw.data[0] = 0xe0;
			kii_handle_input(&event, NULL);
	    
			event.raw.data[0] = 
				raw_emu_e0s[keycode - 96] | ((!value) << 7);
			kii_handle_input(&event, NULL);
	    
			if (keycode == 99) {

				event.raw.data[0] = 0xe0;
				kii_handle_input(&event, NULL);
				event.raw.data[0] = 0x37 | ((!value) << 7);
				kii_handle_input(&event, NULL);	      
			}

		} else if (keycode == 119) {

			event.raw.data[0] = 0xe1;
			kii_handle_input(&event, NULL);
			event.raw.data[0] = 0x1d | ((!value) << 7);
			kii_handle_input(&event, NULL);
			event.raw.data[0] = 0x45 | ((!value) << 7);
			kii_handle_input(&event, NULL);

		} else {

			event.raw.data[0] = keycode |  ((!value) << 7);
			kii_handle_input(&event, NULL);
		}
	}
	
	event.key.size = sizeof(kii_key_event_t);
	event.key.code = keycode;

	switch (value) {
	case 2:	event.key.type = KII_EV_KEY_REPEAT;
		break;
	case 1:	event.key.type = KII_EV_KEY_PRESS;
		break;
	case 0:	event.key.type = KII_EV_KEY_RELEASE;
		break;
	default: KRN_INTERNAL_ERROR;
		return;
	}

	if (kbd->report & (1 << event.key.type)) {

		kii_handle_input(&event, NULL);
	}
}

struct input_handle *kii_kbd_connect(struct input_handler *handler, 
				     struct input_dev *dev)
{
	struct input_handle *handle;
	kii_input_t *ptr;

	/* Test that it's a keyboard
	*/
	if (!test_bit(EV_KEY, dev->evbit) || 
	    !test_bit(KEY_A, dev->keybit) ||
	    !test_bit(KEY_Z, dev->keybit)) 
		goto err0;
  
	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL)))
		goto err0;

	memset(handle, 0, sizeof(struct input_handle));
	if (!(ptr = kmalloc(sizeof(kii_input_t), GFP_KERNEL)))
		goto err1;

	memset(ptr, 0, sizeof(kii_input_t));

	ptr->events = KII_EM_KEY | KII_EM_RAW_DATA;
	ptr->report = KII_EM_KEY | KII_EM_RAW_DATA;
	(kii_input_t *)handle->private = ptr;
	if (kii_register_input((~0), ptr)) goto err2;
	handle->dev = dev;
	handle->handler = handler;
	input_open_device(handle);
	KRN_NOTICE("Adding KII keyboard on input%d", dev->number);
	return handle;

 err2:
	kfree(ptr);
 err1:
	kfree(handle);
 err0:
	return NULL;
}

struct input_kii_ptr_priv
{ 
	kii_u_t           butn;
	kii_s_t           dx, dy, dz;
};

void kii_ptr_event (struct input_handle *handle, unsigned int type,
		    unsigned int code, int value)
{
	struct input_kii_ptr_priv *this;
	kii_event_t event;
	kii_input_t *ptr;
	kii_u_t butnbit;

	add_mouse_randomness(value);

	ptr = (kii_input_t *)(handle->private);
	event.any.focus  = ptr->focus;
	event.any.device = ptr->id;
	event.any.time   = jiffies;

	this = (struct input_kii_ptr_priv *)(ptr->priv.priv_ptr);

	if (type == EV_REL) {

		event.any.type = KII_EV_PTR_RELATIVE;
		event.pmove.y = event.pmove.x = event.pmove.wheel = 0;
		event.pmove.size = sizeof(kii_pmove_event_t);
		event.pmove.dummy[0] = event.pmove.dummy[1] = 0;    
		switch (code) {
		case REL_X:
			this->dx += value;
			event.pmove.x = value;
			break;
		case REL_Y:
			this->dy -= value; 
			event.pmove.y = value;
			break;
		case REL_WHEEL:
			this->dz += value; 
			event.pmove.wheel = value;
			break;
		}
		if (ptr->report & KII_EM_PTR_RELATIVE) {

			KRN_DEBUG(2, "KII_EV_PTR_RELATIVE");
			kii_handle_input(&event,NULL);
			}
		return;
	}
	if (type == EV_KEY) {
		switch (value) {
		case 2: /* Add KII_EV_PTR_BUTTON_REPEAT ? */
		case 1:	event.key.type = KII_EV_PTR_BUTTON_PRESS;
			break;
		case 0:	event.key.type = KII_EV_PTR_BUTTON_RELEASE;
			break;
		default: KRN_INTERNAL_ERROR;
			return;
		}
		switch (code) {
		case BTN_LEFT:		butnbit = 0x0001; break;
		case BTN_RIGHT:		butnbit = 0x0002; break;
		case BTN_MIDDLE:	butnbit = 0x0004; break;
		case BTN_SIDE:		butnbit = 0x0008; break;
		case BTN_EXTRA:		butnbit = 0x0010; break;
		case BTN_FORWARD:	butnbit = 0x0020; break;
		case BTN_BACK:		butnbit = 0x0040; break;
		default:		butnbit = 0x0080;
		}

		if (value) {
			this->butn |= butnbit; 
		} else {
			this->butn &= ~butnbit;
		}
		event.pbutton.state = this->butn;
		if (ptr->report & (1 << event.pbutton.type)) {
			KRN_DEBUG(2, "%s",
				(event.pbutton.type == KII_EV_PTR_BUTTON_PRESS)
				  ? "KII_EV_PTR_BUTTON_PRESS" 
				  : "KII_EV_PTR_BUTTON_RELEASE");
			kii_handle_input(&event, NULL);
		}
	}
}

struct input_handle *kii_ptr_connect(struct input_handler *handler, 
				     struct input_dev *dev)
{
	struct input_handle *handle;
	kii_input_t *ptr;

	if (!(test_bit(EV_KEY, dev->evbit) && test_bit(EV_REL, dev->evbit))) {

		KRN_DEBUG(2, "device doesn't have both rels and keys");
		goto err0;
	}
  
	if (!(test_bit(REL_X, dev->relbit) && test_bit(REL_Y, dev->relbit))) {

		KRN_DEBUG(2, "device isn't a pointer device");
		goto err0;
	}
  
	if (!test_bit(BTN_LEFT, dev->keybit)) {

		KRN_DEBUG(2, "device has no mouse button");
		goto err0;
	}
  
	if (!(handle = kmalloc(sizeof(struct input_handle), GFP_KERNEL))) {

		KRN_DEBUG(2, "failed to allocate input_handle");
		goto err0;
	}
	memset(handle, 0, sizeof(struct input_handle));
	
	if (!(ptr = kmalloc(sizeof (kii_input_t), GFP_KERNEL))) {

		KRN_DEBUG(2, "failed to allocate kii_input_t");
		goto err1;
	}
	memset(ptr, 0, sizeof(kii_input_t));

	if (!(ptr->priv.priv_ptr = kmalloc(sizeof(struct input_kii_ptr_priv), 
					   GFP_KERNEL))) {

		KRN_DEBUG(2, "failed to allocate private data");
		goto err2;
	}
	memset(ptr->priv.priv_ptr, 0, sizeof(struct input_kii_ptr_priv));
  
	ptr->events = KII_EM_POINTER & ~KII_EM_PTR_ABSOLUTE;
	ptr->report = KII_EM_PTR_RELATIVE | KII_EM_PTR_BUTTON;

	(kii_input_t *)(handle->private) = ptr;
	kii_register_input((~0), ptr);
  
	handle->dev = dev;
	handle->handler = handler;
	input_open_device(handle);
	KRN_NOTICE("Adding KII pointer on input%d", dev->number);
	return handle;
 err2:
	kfree(ptr);
 err1:
	kfree(handle);
 err0:
	return NULL;

}

void kii_kbd_disconnect(struct input_handle *handle)
{
	KRN_NOTICE("Removing KII keyboard from input%d", handle->dev->number);
	kii_unregister_input((kii_input_t *)handle->private);
	kfree(handle->private);
	handle->private = NULL;
	input_close_device(handle);
	kfree(handle);
}

void kii_ptr_disconnect(struct input_handle *handle) 
{
	KRN_NOTICE("Removing KII pointer from input%d", handle->dev->number);
	kii_unregister_input((kii_input_t *)handle->private);
	kfree(handle->private);
	handle->private = NULL;
	input_close_device(handle);
	kfree(handle);
}

int focus_init(void)
{
	int nr_focus = 0;
	struct input_handler *kbd, *ptr;

	kbd = (struct input_handler *) kmalloc(sizeof (struct input_handler),
		GFP_KERNEL);
	if (!kbd) {

		KRN_DEBUG(2, "failed to allocate kbd input handler");
		return 0;
	}
	kbd->connect	= kii_kbd_connect;
	kbd->disconnect = kii_kbd_disconnect;
	kbd->event	= kii_kbd_event;

	input_register_handler(kbd); /* Assigns one handle to each kbd. */

	nr_focus = 1;

	ptr = (struct input_handler *)kmalloc(sizeof (struct input_handler),
					      GFP_KERNEL);
	if (!ptr) {

		KRN_DEBUG(2, "failed to allocate pointer input handler");
		return(nr_focus);
	}
	ptr->connect	= kii_ptr_connect;
	ptr->disconnect	= kii_ptr_disconnect;
	ptr->event	= kii_ptr_event;

	input_register_handler(ptr); /* Assigns one handle to each ptr. */

	return nr_focus;
}

static int __init kii_input_linux_init(void)
{
  return 0;
}

static void __exit kii_input_linux_exit(void)
{
}

module_init(kii_input_linux_init);
module_exit(kii_input_linux_exit);
