/* ----------------------------------------------------------------------------
**	XGGI display driver initialization code
** ----------------------------------------------------------------------------
**
**	Copyright (C) 1997	Jason McMullan
**	Copyright (C) 1997-1998	Michael Krause
**	Copyright (C) 1999-2000	Steffen Seeger
**
**	$Log: phoenixOutput.c,v $
**	Revision 1.2  2000/09/21 11:16:49  seeger_s
**	- first somewhat working version
**	
**	Revision 1.1  2000/07/04 11:01:52  seeger_s
**	- added PhoeniX DDX stubs and included in build system
**	
**	Revision 1.2  2000/06/28 20:45:03  seeger_s
**	- fixed headers
**	
**	Revision 1.1  2000/06/25 20:08:08  seeger_s
**	- added input, output and keycode translation code.
**	
*/

#include "scrnintstr.h"
#include "servermd.h"
#include "colormapst.h"
#include "mi/mi.h"
#include "mi/mipointer.h"

#include "ddx/xaa/xf86.h"
#include "ddx/xaa/xaa.h"

#include "types.h"
#include "phoenix.h"

#define	APP_DEBUG	ErrorF

static int phoenix_init_defaults = 1;
phoenix_t phoenix;

static void PhoenixInitDefaults(void)
{
	u_t i;

	APP_DEBUG("PhoenixInitDefaults(): initializing phoenix\n");

	memset(&phoenix, 0, sizeof(phoenix));

	for (i = 0; i < MAXSCREENS; i++) {

		phoenix_output_t *output = phoenix.output + i;

		output->img.virt.x = 640;
		output->img.virt.y = 400;

		output->img.size.x = output->img.virt.x;
		output->img.size.y = output->img.virt.y;

		output->img.frames = 1;

		output->img.iluts  = 1;
		output->img.ilutm  = KGI_AM_COLORS;

		output->img.fam    = KGI_AM_COLOR_INDEX;
		strcpy(output->img.bpfa, KGI_AS_8);

		output->dpi.x = output->dpi.y = 75;
	}

	phoenix.flags |= PHOENIX_F_INIT_OUTPUT_HW | PHOENIX_F_INIT_INPUT_HW;
	phoenix.outputs = 1;
}

/*
**	pointer functions
*/

static Bool PhoenixPointerCursorOffScreen(ScreenPtr *screen, int *x, int *y)
{
	return FALSE;
}

static void PhoenixPointerCrossScreen (ScreenPtr screen, Bool entering)
{
}

#define	PhoenixPointerWarpCursor	miPointerWarpCursor
#define	PhoenixPointerEnqueueEvent	mieqEnqueue
#define	PhoenixPointerNewEventScreen	mieqSwitchScreen

static const miPointerScreenFuncRec phoenix_screen_pointer =
{
	PhoenixPointerCursorOffScreen,
	PhoenixPointerCrossScreen,
	PhoenixPointerWarpCursor,
	PhoenixPointerEnqueueEvent,
	PhoenixPointerNewEventScreen
};

/*
**	screen helper functions
*/

static Bool PhoenixSaveScreen(ScreenPtr screen, int on)
{
	ErrorF("PhoenixSaveScreen(screen=%i, on=%i)\n",
		screen->myNum, on);
	return TRUE;
}

extern void *kgiInitFramebuffer(kgi_u_t sizex, kgi_u_t sizey);

static void PhoenixXAA_Sync(ScrnInfoPtr pScrn)
{

}

static int PhoenixScreenInit(int id, ScreenPtr screen, 
	int idx, char **argv)
{
	phoenix_output_t *output = phoenix.output + idx;

	APP_DEBUG("PhoenixScreenInit(id=%i, screen=%p, idx=%i, ...) "
		"serverGeneration=%i\n",
		id, screen, idx, serverGeneration);

#if 0
	APP_ASSERT(id == screen->myNum);
	APP_ASSERT(argv == NULL);
#endif

	if (phoenix.flags & PHOENIX_F_INIT_OUTPUT_HW) {

#		warning	init output!

		output->fb = kgiInitFramebuffer(output->img.virt.x,
			output->img.virt.y);
		if (NULL == output->fb) {

			APP_DEBUG("failed to allocate frame buffer for "
				"screen %i (index %i)\n", id, idx);
			return FALSE;
		}
		APP_DEBUG("PhoenixScreenInit(): allocated %ix%i frame buffer "
			"@%p for screen %i (index %i)\n",
			output->img.virt.x, output->img.virt.y,
			output->fb, id, idx);
	}

	output->x11 = screen;

	screen->SaveScreen = PhoenixSaveScreen;

	if (! cfbScreenInit(screen, output->fb, 
		output->img.virt.x, output->img.virt.y,
		output->dpi.x, output->dpi.y,
		output->img.virt.x)) {

		APP_DEBUG("cfbScreenInit() failed for screen %i "
			"(index %i).\n", id, idx);
		return FALSE;
	}

	output->xf86 = xf86CreateScreenInfoRec();
	if (! xf86Init(output->x11, output->xf86)) {
	
		APP_DEBUG("failed to initialize XF86 compatibility layer for"
			"screen %i (index %i)\n", id, idx);
		xf86DestroyScreenInfoRec(output->xf86);
		output->xf86 = NULL;
		return FALSE;
	}

	output->xaa = XAACreateInfoRec();
	output->xaa = PhoenixXAA_Sync;
	if (! XAAInit(output->x11, output->xaa)) {

		APP_DEBUG("failed to initialize XAA for screen %i (index %i)\n",
			id, idx);
		XAADestroyInfoRec(output->xaa);
		output->xaa = NULL;
		return FALSE;
	}

	if (! miInitializeBackingStore(screen)) {

		APP_DEBUG("miInitializeBackingStore() failed for screen %i "
			"(index %i).\n", id, idx);
	}

	if (! miDCInitialize(screen, &phoenix_screen_pointer)) {

		APP_DEBUG("miDCIntitialize() failed for screen %i "
			"(index %i).\n", id, idx);
		return FALSE;
	}

	if (! cfbCreateDefColormap(screen)) {

		APP_DEBUG("cfbCreateDefColormap() failed for screen %i "
			"(index %i).\n", id, idx);
		return FALSE;
	}

	return TRUE;
}

static inline u_t PhoenixRoundedBPP(const u_t bpp)
{
#if 0
	APP_ASSERT(0 < bpp);
#endif

	if (bpp <= 4)	return 4;
	if (bpp <= 8)	return 8;
	if (bpp <= 16)	return 16;
	if (bpp <= 24)	return 24;
	if (bpp <= 32)	return 32;

	FatalError("%i bits per pixel are too many.\n", bpp);
}

static inline void PhoenixAddPixmapFormat(ScreenInfo *screenInfo, 
	const u_t depth, const u_t bpp, const u_t pad)
{
	u_t fmt;

	for (fmt = 0; fmt < screenInfo->numPixmapFormats; fmt++) {

		/*	use largest padding required
		*/
		if ((screenInfo->formats[fmt].depth == depth) &&
			(screenInfo->formats[fmt].bitsPerPixel == bpp)) {

			if (screenInfo->formats[fmt].scanlinePad < pad) {

				screenInfo->formats[fmt].scanlinePad = pad;
			}
			return;
		}
	}

	if (MAXFORMATS <= fmt) {

		FatalError("MAXFORMATS = %i is too few for this configuration.\n",
			MAXFORMATS);
	}

	APP_DEBUG("adding format %i: bpp=%i, depth=%i, pad=%i\n",
		screenInfo->numPixmapFormats, bpp, depth, pad);

	screenInfo->formats[fmt].depth	= depth;
	screenInfo->formats[fmt].bitsPerPixel = bpp;
	screenInfo->formats[fmt].scanlinePad = pad;

	screenInfo->numPixmapFormats++;
}

void InitOutput(ScreenInfo *screenInfo, int argc, char *argv[])
{
	u_t i;

	if (phoenix_init_defaults) {

		PhoenixInitDefaults();
		phoenix_init_defaults = 0;
	}

	APP_DEBUG("\nInitOutput(): initializing screens\n");

	screenInfo->imageByteOrder	= IMAGE_BYTE_ORDER;

	/*	bitmap format
	*/
	screenInfo->bitmapScanlineUnit	= BITMAP_SCANLINE_UNIT;
	screenInfo->bitmapScanlinePad	= BITMAP_SCANLINE_PAD;
	screenInfo->bitmapBitOrder	= BITMAP_BIT_ORDER;

#if 0
	/*	we always support the 1bpp pixmap format
	*/
	screenInfo->formats[0].depth		= 1;
	screenInfo->formats[0].bitsPerPixel	= 1;
	screenInfo->formats[0].scanlinePad	= BITMAP_SCANLINE_PAD;
	screenInfo->numPixmapFormats = 1;
#endif

	for (i = 0; i < phoenix.outputs; i++) {

		u_t	color_bpp, index_bpp, mask, attr;

		/*	count index and color bits for per-frame-attributes
		*/
		color_bpp = 0;
		index_bpp = 0;
		attr = 0;

		for (mask = 1; mask & KGI_AM_ALL; mask += mask) {

			if (! (phoenix.output[i].img.fam & mask)) {

				continue;
			}

			if (mask & KGI_AM_COLOR_INDEX) {

				index_bpp += phoenix.output[i].img.bpfa[attr];
			}

			if (mask & KGI_AM_COLORS) {

				color_bpp += phoenix.output[i].img.bpfa[attr];
			}

			attr++;	
		}

		/*	add pixmap format
		*/
		if (color_bpp) {

			/*	If there are color bits per pixel, index bits
			**	control overlay color.
			*/
			PhoenixAddPixmapFormat(screenInfo, color_bpp,
				PhoenixRoundedBPP(color_bpp),
				BITMAP_SCANLINE_PAD);
		} else {

			if (index_bpp > 0) {

				PhoenixAddPixmapFormat(screenInfo, index_bpp,
					PhoenixRoundedBPP(index_bpp),
					BITMAP_SCANLINE_PAD);
			} else {

				FatalError("Neither color nor index bits per "
					"pixel configured for screen %i.\n", i);
			}
		}

		/*	add screen
		*/
		if (-1 == AddScreen(PhoenixScreenInit, i, NULL)) {

			FatalError("Could not add screen %i\n", i);
		}
	}

	phoenix.flags &= ~PHOENIX_F_INIT_OUTPUT_HW;

	APP_DEBUG("InitOutput(): done.\n");
}

void ddxGiveUp(void)
{
	ErrorF("ddxGiveUp()\n");
}

void OsVendorInit(void) 
{
	ErrorF("OsVendorInit()\n");
}

void OsVendorFatalError(void)
{
	ErrorF("OsVendorFatalError()\n");
}


/*	ddxProcessArgument() gets called first, so we have to initialize our
**	global defaults here, if we want options to override the default values.
**	Please note that argument processing is done once at startup and
**	especially not for each server generation.
*/
void ddxUseMsg(void) 
{
	ErrorF("-----------------------------------------------------------\n");
	ErrorF("-mode WxH         set width, height\n");
	ErrorF("-display <name>   ignored for compatibility\n");
}

int ddxProcessArgument (int argc, char *argv[], int i)
{

/*	APP_ASSERT(((serverGeneration == 1) && phoenix_init_defaults) ||
**		((serverGeneration > 1) && phoenix_init_defaults));
*/
	if (phoenix_init_defaults) {

		PhoenixInitDefaults();
		phoenix_init_defaults = 0;
	}

	if (! strcmp(argv[i], "-display")) {

		return 2;
	}

	if (!strcmp(argv[i], "-mode")) {

		unsigned int width, height;

		if(i + 1 >= argc) {

			UseMsg();
		}

		if (2 != sscanf(argv[i+1], "%ux%u", 
			&width, &height)) {

			ErrorF("Invalid mode spec '-mode %s' ignored.\n",
				argv[i+1]);
			UseMsg();

		} else {

			phoenix.output[0].img.virt.x = width;
			phoenix.output[0].img.virt.y = height;
		}

		return 2;
	}

	return 0;
}

#ifdef DDXTIME /* from ServerOSDefines */
CARD32 GetTimeInMillis(void)
{
	struct timeval  tp;

	X_GETTIMEOFDAY(&tp);
	return (tp.tv_sec*1000) + (tp.tv_usec/1000);
}
#endif

