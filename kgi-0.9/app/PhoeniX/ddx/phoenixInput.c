/* -----------------------------------------------------------------------------
**	X server input handling
** -----------------------------------------------------------------------------
**	Copyright (C)	1997		Jason McMullan
**			1997-1998	Michael Krause
**			1998		Steffen Seeger
**
**	$Log: xggiInput.c,v $
**	Revision 1.2  2000/06/28 20:45:03  seeger_s
**	- fixed headers
**	
**	Revision 1.1  2000/06/25 20:08:08  seeger_s
**	- added input, output and keycode translation code.
**	
*/

#include "scrnintstr.h"
#include "inputstr.h"
#include "mi/mipointer.h"
#include "input.h"
#include "mi/mi.h"

#if 0

/*	The routines from translation.c are needed only here. An extra header
**	looks like overkill.
*/
extern int x2unicode(int xkeysym);
extern int unicode2x(int unicode);

#include <gii/gii.h>


static void xggiQueueEvent(gii_event *ev)
{
	xEvent xev;

	xev.u.keyButtonPointer.time = ev->any.time;

	if ((1 << ev->any.type) & GII_EM_KEYBOARD) {

		xev.u.u.detail = ev->key.code;
	
		switch (ev->any.type) {

		case GII_EV_KEY_PRESS:
		case GII_EV_KEY_REPEAT:
			xev.u.u.type = KeyPress;
			break;
	    
		case GII_EV_KEY_RELEASE:
			xev.u.u.type = KeyRelease;
			break;
	    
		default:
			ErrorF("unknown keyboard event type %i\n",
				ev->any.type);
			return;
		}
	
		mieqEnqueue(&xev);
		return;
	}	/* if ((1 << ev->any.type) & GII_EM_KEYBOARD) ... */

	if ((1 << ev->any.type) & GII_EM_POINTER) {

		int dx, dy;
		static int ptr_oldx = 0, ptr_oldy = 0;
    
		if (ev->any.type == GII_EV_PTR_ABSOLUTE) {

			dx = ev->pmove.x - ptr_oldx;      
			dy = ev->pmove.y - ptr_oldy;
			ptr_oldx = ev->pmove.x;
			ptr_oldy = ev->pmove.y;
			miPointerDeltaCursor(dx, dy, ev->any.time);
			return;
		}

		if (ev->any.type == GII_EV_PTR_RELATIVE) {

			ptr_oldx += (dx = ev->pmove.x);
			ptr_oldy += (dy = ev->pmove.y);
			miPointerDeltaCursor(dx, dy, ev->any.time);
			return;
		}

		if ((ev->any.type == GII_EV_PTR_BUTTON_PRESS) || 
			(ev->any.type == GII_EV_PTR_BUTTON_RELEASE)) {

			gii_u button = 2;
			gii_u mask = 1 << button;

			xev.u.u.type = (ev->any.type == GII_EV_PTR_BUTTON_PRESS)
				? ButtonPress : ButtonRelease;

			while (mask) {

				if (ev->pbutton.button & mask) {

					xev.u.u.detail = button;
					mieqEnqueue(&xev);
				}
				mask >>= 1;
				button--;
			}

			return;
		}
		ErrorF("unknown pointer event type %i\n", ev->any.type);
	}	/* if ((1 << ev->any.type) & GII_EM_POINTER) ... */

}
#endif

#if 0 
static int xggiPtrProc(DeviceIntPtr device, int action)
{
	DevicePtr ptr = (DevicePtr) device;
	api_context ctx = (api_context) ptr->devicePrivate;

	BYTE buttonmap[4];

	switch (action) {

	case DEVICE_INIT:
		buttonmap[1] = 1;
		buttonmap[2] = 2;
		buttonmap[3] = 3;
		InitPointerDeviceStruct(ptr, buttonmap, 3,
			miPointerGetMotionEvents,
			(PtrCtrlProcPtr) NoopDDA,
			miPointerGetMotionBufferSize());
		break;
	
	case DEVICE_ON:
		ptr->on = TRUE;
		giiEnable(ctx, GII_DEVICE_POINTER);
		break;
	
	case DEVICE_OFF:
		ptr->on = FALSE;
		giiDisable(ctx, GII_DEVICE_POINTER);
		break;
	
	case DEVICE_CLOSE:
		giiDisable(ctx, GII_DEVICE_POINTER);
		break;
	}

	return Success;
}
#endif


Bool LegalModifier(unsigned int key, DevicePtr kbd)
{
#if 0
	return	giiLegalModifier((api_context) kbd->devicePrivate,
			GII_DEVICE_KEYBOARD, key) ? TRUE : FALSE;
#endif
	return TRUE;
}

#if 0
int xggiKbdProc(DeviceIntPtr device, int action)
{
	DevicePtr kbd =	(DevicePtr) device;
	api_context ctx = (api_context) kbd->devicePrivate;

	KeySymsRec keymap;
	CARD8 modmap[MAP_LENGTH];
	gii_u foo, i, j, kmap[256];
    
	switch (action) {

	case DEVICE_INIT:
		/*	first read out the 'keyboard' parameters
		**	and allocate a local keymap
		*/
		giiGetuv(ctx, GII_KBD_MIN_KEYCODE, &foo, 1);
		keymap.minKeyCode = foo;

		giiGetuv(ctx, GII_KBD_MAX_KEYCODE, &foo, 1);
		keymap.maxKeyCode = foo;

		giiGetuv(ctx, GII_KBD_MAX_MAPSIZE, &foo, 1);
		keymap.mapWidth = foo;

		if ((keymap.minKeyCode > 255) || (keymap.maxKeyCode > 255)) {
	
			ErrorF("don't know how to handle keycodes > 255\n");
			return -0;
		}

		foo = (keymap.maxKeyCode - keymap.minKeyCode) * keymap.mapWidth;
		keymap.map = ALLOCATE_LOCAL(foo);
		if (NULL == keymap.map) {

			ErrorF("failed to allocate temporary keymap buffer\n");
			return -1;
		}

		/*	now read out each look-up-table and convert to XKeySyms
		*/
		for (i = keymap.mapWidth; --i; ) {

			giiGetuv(ctx, GII_KBD_KEYMAP(i), kmap, 256);

			for (j = keymap.minKeyCode; j < keymap.maxKeyCode;
				j++) {

				keymap.map[((j-keymap.minKeyCode) * 
					keymap.mapWidth) + i] =
					unicode2x(kmap[j]);
			}
		}

		/*	Finally build the modifier table. Only care about
		**	normal modifiers, but clear all modifier XKeySyms.
		*/
		for (j = keymap.minKeyCode; j < keymap.maxKeyCode; j++) {

			if (GII_IS_MODIFIER(kmap[j])) {

				for (i = 0; i < (unsigned) keymap.mapWidth;
					i++) {

					keymap.map[(j-keymap.minKeyCode) *
						keymap.mapWidth + i] =
						NoSymbol;
				}
			}

			if (GII_IS_NORMAL_MODIFIER(kmap[j])) {

				if (GII_MODIFIER(kmap[j]) < 8) {

					modmap[GII_MODIFIER(kmap[j])] = j;
				} 
			}
		}

		InitKeyboardDeviceStruct(kbd, &keymap, modmap,
			(BellProcPtr) NoopDDA, (KbdCtrlProcPtr) NoopDDA);
		DEALLOCATE_LOCAL(keymap.map);
		break;

	case DEVICE_ON: 
		kbd->on = TRUE;
		giiEnable(ctx, GII_DEVICE_KEYBOARD);
		break;

	case DEVICE_OFF: 
		kbd->on = FALSE;
		giiDisable(ctx, GII_DEVICE_KEYBOARD);
		break;

	case DEVICE_CLOSE:
		kbd->on = FALSE;
		giiDisable(ctx, GII_DEVICE_KEYBOARD);
		break;
	}

	return Success;
#undef kbd
}
#endif



void ProcessInputEvents()
{
	mieqProcessInputEvents();
	miPointerUpdate();
}

void AbortDDX()
{
#if 0
	ggiExit();
#endif
}

static CARD32 xggiSnarf(OsTimerPtr timer, CARD32 time, pointer data)
{
#if 0
	api_context ctx = (api_context) data;
	gii_event *ev;
    
	while (giiGetEvent(ctx, &ev, GII_EM_POINTER | GII_EM_KEYBOARD)) {

		xggiQueueEvent(ev);
	}
	return 20 /* milliseconds */;
#endif
}

/*
**	Initailize input handlers
*/
void InitInput(int argc, char *argv[])
{
#if 0
	api_context xggi_input = apiInit("GII", NULL);
	DeviceIntPtr ptr, kbd;
	static OsTimerPtr timer;

	if (NULL == xggi_input) {

		FatalError("failed to initialize input API");
	}

	kbd = AddInputDevice(xggiKbdProc, TRUE);
	((DevicePtr) kbd)->devicePrivate = (pointer) xggi_input;
	RegisterKeyboardDevice(kbd);

	ptr = AddInputDevice(xggiPtrProc, TRUE);
	((DevicePtr) ptr)->devicePrivate = (pointer) xggi_input;
	RegisterPointerDevice(ptr);

	miRegisterPointerDevice(screenInfo.screens[0], ptr);
	mieqInit((DevicePtr) kbd, (DevicePtr) ptr);

	timer = TimerSet(NULL, 0, 20 /* milliseconds */, xggiSnarf, xggi_input);
#endif
}
