/* -----------------------------------------------------------------------------
**	PhoeniX server input handling
** -----------------------------------------------------------------------------
**	Copyright (C)	1997		Jason McMullan
**			1997-1998	Michael Krause
**			1998-2000	Steffen Seeger
**
**	$Log: phoenixInput.c,v $
**	Revision 1.1  2000/07/04 11:01:52  seeger_s
**	- added PhoeniX DDX stubs and included in build system
*/

#include "scrnintstr.h"
#include "inputstr.h"
#include "mi/mipointer.h"
#include "input.h"
#include "mi/mi.h"

/*	The routines from translation.c are needed only here.
**	An extra header looks like overkill...
*/
extern int x2unicode(int xkeysym);
extern int unicode2x(int unicode);

#include "types.h"
#include "phoenix.h"

#include <X11/keysym.h>

#define	APP_DEBUG	ErrorF


static int PhoenixPtrProc(DeviceIntPtr device, int action)
{
	DevicePtr ptr = (DevicePtr) device;

	BYTE buttonmap[4];

	switch (action) {

	case DEVICE_INIT:
		buttonmap[1] = 1;
		buttonmap[2] = 2;
		buttonmap[3] = 3;
		if (! InitPointerDeviceStruct(ptr, buttonmap, 3,
			miPointerGetMotionEvents,
			(PtrCtrlProcPtr) NoopDDA,
			miPointerGetMotionBufferSize())) {

			APP_DEBUG("PhoenixPtrProc(): InitPointerDeviceStruct failed\n");
			return !Success;
		}
		APP_DEBUG("PhoenixPtrProc(): pointer initialized "
			"(buttons=%i)\n", 3);
		break;

	case DEVICE_ON:
		if (! phoenix.input.enabled++) {

			AddEnabledDevice(kiiEventDeviceFD(phoenix.input.kii));
		}
		ptr->on = TRUE;
		APP_DEBUG("PhoenixPtrProc(): pointer enabled\n");
		break;
	
	case DEVICE_OFF:
	case DEVICE_CLOSE:
		ptr->on = FALSE;
		if (! --phoenix.input.enabled) {

			RemoveEnabledDevice(kiiEventDeviceFD(phoenix.input.kii));
		}
		APP_DEBUG("PhoenixPtrProc(): pointer %s\n",
			(action == DEVICE_OFF) ? "disabled" : "closed");
		break;
	}

	return Success;
}

int PhoenixKbdProc(DeviceIntPtr device, int action)
{
	DevicePtr kbd =	(DevicePtr) device;
	KeySymsRec keymap;
	CARD8 modmap[MAP_LENGTH];
	kii_u_t foo, i, j;
	kii_unicode_t kmap[256];
    
	switch (action) {

	case DEVICE_INIT:
		/*	first read out the 'keyboard' parameters
		**	and allocate a local keymap
		*/
		kiiGetu(phoenix.input.kii, KII_KBD_MIN_KEYCODE, &foo);
		keymap.minKeyCode = foo + 8;

		kiiGetu(phoenix.input.kii, KII_KBD_MAX_KEYCODE, &foo);
		keymap.maxKeyCode = foo + 8;

		kiiGetu(phoenix.input.kii, KII_KBD_MAX_MAPSIZE, &foo);
		keymap.mapWidth = foo;

		if ((keymap.minKeyCode > 255) || (keymap.maxKeyCode > 255)) {
	
			ErrorF("don't know how to handle keycodes > 255\n");
			return !Success;
		}

		foo = (keymap.maxKeyCode - keymap.minKeyCode + 1) *
			keymap.mapWidth;
		keymap.map = ALLOCATE_LOCAL(foo*sizeof(keymap.map[0]));
		if (NULL == keymap.map) {

			ErrorF("failed to allocate temporary keymap buffer\n");
			return !Success;
		}

		/*	now read out each look-up-table and convert to XKeySyms
		*/
		for (i = keymap.mapWidth; i--; ) {

			ErrorF("reading map %i\n", i);

			if (KII_EOK != kiiGetKeymap(phoenix.input.kii, kmap,
				i, keymap.minKeyCode-8, keymap.maxKeyCode-8)) {

				ErrorF("Failed to read keymap %i\n", i);
				exit(1);
				DEALLOCATE_LOCAL(keymap.map);
				return !Success;
			}

			for (j = keymap.minKeyCode; j <= keymap.maxKeyCode;
				j++) {

				kii_u_t index = ((j-keymap.minKeyCode) * 
					keymap.mapWidth) + i;
				if ((keymap.map[index] = unicode2x(kmap[j-8]))
					== -1) {

					keymap.map[index] = NoSymbol;
				}
				ErrorF("mapped key code %i, unicode %4x, keysym %8x\n",
					j, kmap[j-8], keymap.map[index]);
			}
		}

		/*	Finally build the modifier table. Only care about
		**	normal modifiers, but clear all modifier XKeySyms.
		*/
		for (i = 0; i < MAP_LENGTH; i++) {

			modmap[i] = NoSymbol;
		}
		for (i = keymap.minKeyCode; i < keymap.maxKeyCode; i++) {

			kii_u_t index = (i-keymap.minKeyCode)*keymap.mapWidth;

			switch (keymap.map[index]) {

			case XK_Shift_L:
			case XK_Shift_R:     modmap[i] = ShiftMask; break;

			case XK_Caps_Lock:   modmap[i] = LockMask; break;

			case XK_Control_L:
			case XK_Control_R:   modmap[i] = ControlMask; break;

			case XK_Alt_L:
			case XK_Alt_R:       modmap[i] = Mod1Mask; break;

			case XK_Num_Lock:    modmap[i] = Mod2Mask; break;

			case XK_Meta_L:
			case XK_Meta_R:      modmap[i] = Mod3Mask; break;

			case XK_Super_L:
			case XK_Super_R:
			case XK_Kana_Lock:
			case XK_Kana_Shift:  modmap[i] = Mod4Mask; break;

			case XK_Hyper_L:
			case XK_Hyper_R:
			case XK_Scroll_Lock: modmap[i] = Mod5Mask; break;

			default:
				continue;
			}

			ErrorF("modifier key: code %.3i, keysym %4x, modmap %2x\n",
				i, keymap.map[index], modmap[i]);

			for (j = 0; j < (unsigned) keymap.mapWidth; j++) {

				keymap.map[index+j] = keymap.map[index];
			}
		}

		if (! InitKeyboardDeviceStruct(kbd, &keymap, modmap,
			(BellProcPtr) NoopDDA, (KbdCtrlProcPtr) NoopDDA)) {

			DEALLOCATE_LOCAL(keymap.map);
			APP_DEBUG("PhoenixKbdProc(): InitKeyboardDeviceStruct failed\n");
			return !Success;
		}

		DEALLOCATE_LOCAL(keymap.map);
		APP_DEBUG("PhoenixKbdProc(): keyboard initialized "
			"(minKeyCode=%i maxKeyCode=%i mapWidth=%i)\n",
			keymap.minKeyCode, keymap.maxKeyCode, keymap.mapWidth);
		break;

	case DEVICE_ON: 
		if (! phoenix.input.enabled++) {

			AddEnabledDevice(kiiEventDeviceFD(phoenix.input.kii));
		}
		kbd->on = TRUE;
		APP_DEBUG("PhoenixKbdProc(): keyboard enabled\n");
		break;

	case DEVICE_OFF: 
	case DEVICE_CLOSE:
		kbd->on = FALSE;
		if (! --phoenix.input.enabled) {

			RemoveEnabledDevice(kiiEventDeviceFD(phoenix.input.kii));
		}
		APP_DEBUG("PhoenixKbdProc(): keyboard %s\n",
			(action == DEVICE_OFF) ? "disabled" : "closed");
		break;
	}

	return Success;
}

Bool LegalModifier(unsigned int key, DevicePtr kbd)
{
	return	kiiLegalModifier(phoenix.input.kii, KII_DEVICE_KEYBOARD, key)
			? TRUE : FALSE;
}

void ProcessInputEvents()
{
	/* APP_DEBUG("ProcessInputEvents(): processing input events\n"); */
	
	while (kiiEventAvailable(phoenix.input.kii)) {

		xEvent xev;
		const kii_event_t *ev = kiiNextEvent(phoenix.input.kii);

		xev.u.keyButtonPointer.time = ev->any.time;

		if ((1 << ev->any.type) & KII_EM_KEYBOARD) {

			APP_DEBUG("keyboard event\n");

			xev.u.u.detail = ev->key.code + 8;
	
			switch (ev->any.type) {

			case KII_EV_KEY_PRESS:
			case KII_EV_KEY_REPEAT:
				xev.u.u.type = KeyPress;
				mieqEnqueue(&xev);
				break;
	    
			case KII_EV_KEY_RELEASE:
				xev.u.u.type = KeyRelease;
				mieqEnqueue(&xev);
				break;
	    
			default:
				ErrorF("unknown keyboard event type %i\n",
					ev->any.type);
			}

			continue;
		}

		if ((1 << ev->any.type) & KII_EM_POINTER) {

			int dx, dy;
			static int ptr_oldx = 0, ptr_oldy = 0;

			APP_DEBUG("pointer event\n");
    
			if (ev->any.type == KII_EV_PTR_ABSOLUTE) {

				dx = ev->pmove.x - ptr_oldx;      
				dy = ev->pmove.y - ptr_oldy;
				ptr_oldx = ev->pmove.x;
				ptr_oldy = ev->pmove.y;
				miPointerDeltaCursor(dx, dy, ev->any.time);
				continue;
			}

			if (ev->any.type == KII_EV_PTR_RELATIVE) {

				ptr_oldx += (dx = ev->pmove.x);
				ptr_oldy += (dy = ev->pmove.y);
				miPointerDeltaCursor(dx, dy, ev->any.time);
				continue;
			}

			if ((ev->any.type == KII_EV_PTR_BUTTON_PRESS) || 
				(ev->any.type == KII_EV_PTR_BUTTON_RELEASE)) {

				kii_u_t button = 3;
				kii_u_t mask = 1 << button;

				xev.u.u.type = (ev->any.type ==
					KII_EV_PTR_BUTTON_PRESS)
						? ButtonPress : ButtonRelease;

				while (mask) {

					if (ev->pbutton.button & mask) {

						xev.u.u.detail = button;
						mieqEnqueue(&xev);
					}
					mask >>= 1;
					button--;
				}

				continue;
			}
			ErrorF("unknown pointer event type %i\n", 
				ev->any.type);
		}
	}

	mieqProcessInputEvents();
	miPointerUpdate();
}

void AbortDDX()
{
	ErrorF("AbordDDX(): aborting ddx\n");
}


/*
**	Initailize input handlers
*/
void InitInput(int argc, char *argv[])
{
	DeviceIntPtr ptr, kbd;

	APP_DEBUG("InitInput(): initializing input\n");

	if (phoenix.flags & PHOENIX_F_INIT_INPUT_HW) {

		if (kiiInit(&phoenix.input.kii) != KII_EOK) {

			FatalError("kiiInit() failed.\n");
		}
		if (kiiMapDevice(phoenix.input.kii) != KII_EOK) {

			FatalError("kiiMapDevice() failed.\n");
		}
		APP_DEBUG("InitInput(): kii initialized.\n");
	}

	kbd = AddInputDevice(PhoenixKbdProc, TRUE);
	RegisterKeyboardDevice(kbd);

	ptr = AddInputDevice(PhoenixPtrProc, TRUE);
	RegisterPointerDevice(ptr);
	miRegisterPointerDevice((ScreenPtr) NULL, ptr);

	/*	initialize MI event queueing code
	**
	**	mieqInit() sets the input check pointers such that Dispatch()
	**	decides if to call ProcessInputEvents() upon a comparison of
	**	the MI event queue head and tail. These will only be altered
	**	by a call to mieqEnqueue(), which we do from 
	**	ProcessInputEvents() when reading the events. So, we need to
	**	override the input check, otherwise nothing will happen. 
	*/
	mieqInit((DevicePtr) kbd, (DevicePtr) ptr);
	phoenix.always_check[0] = 0;
	phoenix.always_check[1] = 1;
	SetInputCheck(phoenix.always_check, phoenix.always_check + 1);

	phoenix.flags &= ~PHOENIX_F_INIT_INPUT_HW;
	APP_DEBUG("InitInput(): done\n");
}
