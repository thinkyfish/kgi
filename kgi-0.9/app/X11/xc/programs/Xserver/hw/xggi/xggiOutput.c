/* ----------------------------------------------------------------------------
**	XGGI display driver initialization code
** ----------------------------------------------------------------------------
**
**	Copyright (C) 1997	Jason McMullan
**	Copyright (C) 1997-1998	Michael Krause
**	Copyright (C) 1999-2000	Steffen Seeger
**
**	$Log: xggiOutput.c,v $
**	Revision 1.1  2000/02/05 22:51:30  seeger_s
**	- first port from old XGGI experimental server
**	
*/

#include "scrnintstr.h"
#include "servermd.h"
#include "colormapst.h"
#include "mipointer.h"

#include "xf86.h"


#define MAX_COLORS	256 /* For the PseudoColor modes.. */

ggi_context xggiOutput = NULL;

struct ScrnInfo xggiInfo
{
	/*
	**	XAA
	*/
	"XGGI"			/* name				*/
	{ 0, }, 		/* options			*/

	8,			/* bitsPerPixel			*/
	640, 480,		/* displayWidth, displayHeight	*/
	640, 480,		/* virtualX, virtualY		*/
	0,			/* videoRam (detected)		*/

	/*
	**	XGGI private
	*/
	0,			/* displayStride		*/
	0,			/* displayDepth			*/

	NULL			/* pfbMemory			*/
};

static Bool xggiPixmapDepths[33];





void xggiOpenConsole(void)
{
	ErrorF("xggiOpenConsole()\n");
	if (NULL == xggiOutput) {

		ErrorF("initializing GGI, ");
		ggiInit();
		xggiOutput = ggiAPIInit("GGI", NULL);
		if (NULL == xggiOutput) {

			FatalError("failed.\n");
		}
		ErrorF("succeeded\n");
	}
}

static int xggiBitsPerPixel(int depth)
{
	     if (depth == 1) {	return 1;
	else if (depth <= 8)	return 8;
	else if (depth <= 16)	return 16;
	else return 32;
}

void ddxGiveUp(void)
{
	ErrorF("ddxGiveUp()\n");
	/*	clean up the framebuffers
	*/

	if (xggiOutput) {

	    	ggiAPIDone(xggiOutput, NULL);
		xggiOutput = NULL;
	}
}

void OsVendorInit(void) 
{
	ErrorF("OsVendorInit()\n");
}

void OsVendorFatalError(void)
{
	ErrorF("OsVendorFatalError()\n");
}

void ddxUseMsg(void) 
{
	ErrorF("-ggimode WxHxD         set width, height, depth (bpp)\n");
	ErrorF("-keymap file           set key mapping table (required)\n");
	ErrorF("-dynkeys               enable dynamic keycode mapping\n");
}

int ddxProcessArgument (int argc, char *argv[], int i)
{
	if(!strcmp(argv[i], "-ggimode")) {

		if(i + 1 >= argc) {

			UseMsg();
		}

		if (3 != sscanf(argv[i+1], "%dx%dx%d", &xggiScreen.width,
			&xggiScreen.height, &xggiScreen.depth)) {

			xggiScreen.width  = XGGI_DEFAULT_WIDTH;
			xggiScreen.height = XGGI_DEFAULT_HEIGHT;
			xggiScreen.depth  = XGGI_DEFAULT_DEPTH;

			ErrorF("Invalid mode spec `-ggimode %s'\n", argv[i+1]);
			UseMsg();
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

static ColormapPtr xggiInstalledMaps[MAXSCREENS];

static int xggiListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps)
{
	/*	By the time we are processing requests, we can guarantee
	**	that there is always a colormap installed
	*/
	*pmaps = xggiInstalledMaps[pScreen->myNum]->mid;
	return 1;
}


static void xggiInstallColormap(ColormapPtr pmap)
{
	int index = pmap->pScreen->myNum;
	ColormapPtr oldpmap = InstalledMaps[index];

	if (pmap != oldpmap) {

		int entries;
		VisualPtr pVisual;
		Pixel *     ppix;
		xrgb *      prgb;
		xColorItem *defs;
		int i;

		if(oldpmap != (ColormapPtr)None) {

			WalkTree(pmap->pScreen, TellLostMap, 
				(char *)&oldpmap->mid);
		}

		InstalledMaps[index] = pmap;
		WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

		entries = pmap->pVisual->ColormapEntries;
		pVisual = pmap->pVisual;

		ppix = (Pixel *)ALLOCATE_LOCAL(entries*sizeof(Pixel));
		prgb = (xrgb *)ALLOCATE_LOCAL(entries*sizeof(xrgb));
		defs = (xColorItem *)ALLOCATE_LOCAL(entries*sizeof(xColorItem));

		for (i = 0; i < entries; i++) {

			ppix[i] = i;
		}
		/* XXX truecolor */
		QueryColors(pmap, entries, ppix, prgb);

		for (i = 0; i < entries; i++) { /* convert xrgbs to xColorItems */
		    defs[i].pixel = ppix[i] & 0xff; /* change pixel to index */
		    defs[i].red   = prgb[i].red;
		    defs[i].green = prgb[i].green;
		    defs[i].blue  = prgb[i].blue;
		    defs[i].flags = DoRed|DoGreen|DoBlue;
		}
		(*pmap->pScreen->StoreColors)(pmap, entries, defs);

		DEALLOCATE_LOCAL(ppix);
		DEALLOCATE_LOCAL(prgb);
		DEALLOCATE_LOCAL(defs);
	}
}

static void xggiUninstallColormap(ColormapPtr pmap)
{
	ColormapPtr curpmap = InstalledMaps[pmap->pScreen->myNum];

	if(pmap == curpmap) {

		if (pmap->mid != pmap->pScreen->defColormap) {

			curpmap = (ColormapPtr) LookupIDByType(
				pmap->pScreen->defColormap, RT_COLORMAP);
			(*pmap->pScreen->InstallColormap)(curpmap);
		}
	}
}

static void xggiStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
	int i;
	xColorItem directDefs[256];
	ggi_color cmap[MAX_COLORS];

	if (pmap != InstalledMaps[pmap->pScreen->myNum]) {

		return;
	}

	if ((pmap->pVisual->class | DynamicClass) == DirectColor) {

		return;
	}

	ggiGetPaletteVec(MASTER_VIS,0,MAX_COLORS,cmap);

	for (i = 0; i < ndef; i++) {

		if (pdefs[i].pixel >= MAX_COLORS) {

			continue;
		}

		if (pdefs[i].flags & DoRed) {

			cmap[pdefs[i].pixel].r=pdefs[i].red;
		}
		if (pdefs[i].flags & DoGreen) {

			cmap[pdefs[i].pixel].g=pdefs[i].green;
		}
		if (pdefs[i].flags & DoBlue) {

			cmap[pdefs[i].pixel].b=pdefs[i].blue;
		}
	}

	ggiSetPaletteVec(MASTER_VIS,0,MAX_COLORS,cmap);
}

static Bool xggiSaveScreen(ScreenPtr pScreen, int on)
{
	return TRUE;
}

static Bool xggiCloseScreen(int index, ScreenPtr pScreen)
{
	if ((PixmapPtr)pScreen->devPrivate) {

		return (*pScreen->DestroyPixmap)(
			(PixmapPtr) pScreen->devPrivate);
	}
	return TRUE;
}

static char *xggiAllocateFramebufferMemory(void)
{
	int mode;
	ggi_directbuffer_t buf = NULL;
	ggi_pixellinearbuffer *plb;

	if (xggiScreen.pfbMemory) {

		return xggiScreen.pfbMemory; /* already done */
	}

	switch (xggiScreen.depth) {

	case 8:
		mode = GT_8BIT;
		break;

	case 16:
		mode = GT_16BIT;
		break;

	case 24:
		mode = GT_24BIT;
		break;

	case 32:
		mode = GT_32BIT;
		break;
	default:
		ErrorF("XGGI doesn't support %d depth screens.\n",
			xggiScreen.depth);
		return NULL;
	}

/*    ggiSetInfoFlags(MASTER_VIS, GGIFLAG_ASYNC); */

	ErrorF("Setting graphics mode...");
	if (ggiSetGraphMode(MASTER_VIS, xggiScreen.width, xggiScreen.height,
		xggiScreen.width, xggiScreen.height, mode)) {

		ErrorF("Can't set required mode.\n");
		return NULL;
	}
	ErrorF("ok\n");

	ErrorF("Checking direct buffers..\n");
	while (!ggiDBGetBuffer(MASTER_VIS, &buf) && buf) {

		switch(ggiDBGetLayout(buf)) {
		case blPixelLinearBuffer: 
			ErrorF("--> blPixelLinearBuffer\n");
			plb = ggiDBGetPLB(buf);
	
			if(plb->stride < 0) {

				ErrorF("    stride negative: %d\n",plb->stride);
				break;
			}
	
			if ((8 * plb->stride % plb->bpp) != 0) {

				ErrorF("    stride is not a multiple of "
					"bytes per pixel\n");
				break;
			}

			if (plb->origin.x != 0 || plb->origin.y != 0) {

				ErrorF("    origin != 0: %d/%d\n",
					plb->origin.x, plb->origin.y);
				break;
			}
	
			if (plb->bpp != xggiScreen.depth) {

				ErrorF("    bpp (%d) != depth (%d)\n",
					plb->bpp, xggiScreen.depth);
				break;
			}

			if (plb->swap != 0) {

				ErrorF("    swap != 0: %d\n", plb->swap);
				break;
			}

			if (plb->read != plb->write) {

				ErrorF("    writebuffer != readbuffer\n");
				break;
			}

			if (plb->page_size) {

				ErrorF("    Note: paged buffer - XGGI "
					"might not be fun!\n");
			}

			ErrorF("    ** Buffer okay.\n");
			xggiScreen.pfbMemory = plb->read;
			xggiScreen.stride = 8 * plb->stride / plb->bpp;
			break;

		default:
			ErrorF("Layout: Unknown\n");
			break;
		}

		if(! xggiScreen.pfbMemory) {

			ErrorF("You might want to contact the author "
				" so support for this mode can be added.\n\n");
		} else {

			break;
		}
    	}

	if(!xggiScreen.pfbMemory) {

		ErrorF("Sorry, I can't find a suitable frame buffer "
			"for the specified mode.\n");
	}

	return xggiScreen.pfbMemory;
}


static Bool xggiCursorOffScreen (ScreenPtr ppScreen, int *x, int *y)
{
	return FALSE;
}

static void xggiCrossScreen (ScreenPtr pScreen, Bool entering)
{
}

static miPointerScreenFuncRec xggiPointerCursorFuncs =
{
	xggiCursorOffScreen,
	xggiCrossScreen,
	miPointerWarpCursor
};

static Bool xggiFBInitProc(int index, ScreenPtr pScreen, int argc, char *argv[])
{
	const int dpix = 100, dpiy = 100;
	int ret;
	char *pbits;

	xggiScreen.bitsPerPixel = xggiBitsPerPixel(xggiScreen.depth);

	if(!(pbits = xggiAllocateFramebufferMemory())) {

		return FALSE;
	}

	if (!xggiScreenInit(pScreen, pbits, xggiScreen.width, xggiScreen.height,
		dpix, dpiy, xggiScreen.stride)) {

		return FALSE;
	}

	pScreen->CloseScreen = xggiCloseScreen;
	pScreen->SaveScreen = xggiSaveScreen;

	switch(xggiScreen.depth) {
	case 16:
	case 24:
	case 32:
		pScreen->InstallColormap = cfbInstallColormap;
		pScreen->UninstallColormap = cfbUninstallColormap;
		pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
		pScreen->StoreColors = (void (*)())NoopDDA;
		break;
	default:
		pScreen->InstallColormap = xggiInstallColormap;
		pScreen->UninstallColormap = xggiUninstallColormap;
		pScreen->ListInstalledColormaps = xggiListInstalledColormaps;
		pScreen->StoreColors = xggiStoreColors;
		break;
	}

	miDCInitialize(pScreen, &xggiPointerCursorFuncs);

	return cfbCreateDefColormap(pScreen);
} 


void InitOutput(ScreenInfoPtr screenInfo, int argc, char *argv[])
{
	int i;
	int NumFormats = 0;

	xggiOpenConsole();

	for (i = 0; i <= 32; i++) {

		xggiPixmapDepths[i] = FALSE;
	}
	xggiPixmapDepths[1] = TRUE;
	xggiPixmapDepths[xggiScreen.depth] = TRUE;

	for (i = 1; i <= 32; i++) {

		if (xggiPixmapDepths[i]) {

			if (screenInfo->numFormats >= MAXFORMATS) {

				FatalError ("MAXFORMATS is too small "
					"for this server\n");
			}
			screenInfo->formats[screenInfo->numFormats].depth = i;
			screenInfo->formats[screenInfo->numFormats].bitsPerPixel
				= xggiBitsPerPixel(i);
			screenInfo->formats[screenInfo->NumFormats].scanlinePad
				= BITMAP_SCANLINE_PAD;
			NumFormats++;
		}
	}

	screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
	screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
	screenInfo->numPixmapFormats = NumFormats;

	/*	initialize screens
	*/
	if (-1 == AddScreen(xggiFBInitProc, argc, argv)) {

		FatalError("Couldn't add screen", i);
	}
}







