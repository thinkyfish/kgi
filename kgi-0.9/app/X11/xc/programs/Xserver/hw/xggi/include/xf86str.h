/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86str.h,v 1.47 1999/08/22 05:57:31 dawes Exp $ */

/*
 * Copyright (c) 1997 by The XFree86 Project, Inc.
 */

/*
 * This file contains definitions of the public XFree86 data structures/types.
 * Any data structures that video drivers need to access should go here.
 */

#ifndef _XF86STR_H
#define _XF86STR_H

#include "misc.h"
#include "input.h"
#include "scrnintstr.h"
#include "xf86Module.h"

typedef struct { CARD32 red, green, blue; } rgb;

/* Flags for driver messages */
typedef enum {
    X_PROBED,                   /* Value was probed */
    X_CONFIG,                   /* Value was given in the config file */
    X_DEFAULT,                  /* Value is a default */
    X_CMDLINE,                  /* Value was given on the command line */
    X_NOTICE,                   /* Notice */
    X_ERROR,                    /* Error message */
    X_WARNING,                  /* Warning message */
    X_INFO,                     /* Informational message */
    X_NONE                      /* No prefix */
} MessageType;

typedef struct _confscreenrec {
    char *		id;
    int			screennum;
    int			defaultdepth;
    int			defaultbpp;
    int			defaultfbbpp;
} confScreenRec, *confScreenPtr;

typedef struct _screenlayoutrec {
    confScreenPtr	screen;
    confScreenPtr	top;
    confScreenPtr	bottom;
    confScreenPtr	left;
    confScreenPtr	right;
} screenLayoutRec, *screenLayoutPtr;

typedef struct _serverlayoutrec {
    char *		id;
    screenLayoutPtr	screens;
} serverLayoutRec, *serverLayoutPtr;

/* These values should be adjusted when new fields are added to ScrnInfoRec */
#define NUM_RESERVED_INTS		16
#define NUM_RESERVED_POINTERS		16
#define NUM_RESERVED_FUNCS		16

typedef pointer (*funcPointer)(void);

/* flags for SaveRestoreImage */
typedef enum {
    SaveImage,
    RestoreImage,
    FreeImage
} SaveRestoreFlags;

/* DGA */

typedef struct {
   int num;		/* A unique identifier for the mode (num > 0) */
   int flags;		/* DGA_CONCURRENT_ACCESS, etc... */
   int imageWidth;	/* linear accessible portion (pixels) */
   int imageHeight;
   int pixmapWidth;	/* Xlib accessible portion (pixels) */
   int pixmapHeight;	/* both fields ignored if no concurrent access */
   int bytesPerScanline; 
   int byteOrder;	/* MSBFirst, LSBFirst */
   int depth;		
   int bitsPerPixel;
   unsigned long red_mask;
   unsigned long green_mask;
   unsigned long blue_mask;
   short visualClass;
   int viewportWidth;
   int viewportHeight;
   int xViewportStep;	/* viewport position granularity */
   int yViewportStep;
   int maxViewportX;	/* max viewport origin */
   int maxViewportY;
   int viewportFlags;	/* types of page flipping possible */
   int offset;		/* offset into physical memory */
   unsigned char *address;	/* server's mapped framebuffer */
   int reserved1;
   int reserved2;
} DGAModeRec, *DGAModePtr;

typedef struct {
   DGAModePtr mode;
   PixmapPtr pPix;
} DGADeviceRec, *DGADevicePtr;

/*
 * ScrnInfoRec
 *
 * There is one of these for each screen, and it holds all the screen-specific
 * information.
 *
 * Note: the size and layout must be kept the same across versions.  New
 * fields are to be added in place of the "reserved*" fields.  No fields
 * are to be dependent on compile-time defines.
 */

typedef struct _ScrnInfoRec {
    ScreenPtr		pScreen;		/* Pointer to the ScreenRec */
    int			scrnIndex;		/* Number of this screen */

    /* Display-wide screenInfo values needed by this screen */
    int			imageByteOrder;
    int			bitmapScanlineUnit;
    int			bitmapScanlinePad;
    int			bitmapBitOrder;
    int			numFormats;
    PixmapFormatRec	formats[MAXFORMATS];

    int			bitsPerPixel;		/* fb bpp */
    int			depth;			/* depth of default visual */
    rgb			weight;			/* r/g/b weights */
    rgb			mask;			/* rgb masks */
    rgb			offset;			/* rgb offsets */
    int			rgbBits;		/* Number of bits in r/g/b */
    int			defaultVisual;		/* default visual class */
    int			virtualX;		/* Virtual width */
    int			virtualY; 		/* Virtual height */
    int			displayWidth;		/* memory pitch */
    int			frameX0;		/* viewport position */
    int			frameY0;
    int			frameX1;
    int			frameY1;

    confScreenPtr	confScreen;		/* Screen config info */
    int			widthmm;		/* physical display dimensions
						 * in mm */
    int			heightmm;
    int			xDpi;			/* width DPI */
    int			yDpi;			/* height DPI */
    pointer		driverPrivate;		/* Driver private area */
    DevUnion		*privates;		/* other driver privates may
						** hook in here */

    int			colorKey;
    int			overlayFlags;

    /* Some of these may be moved out of here into the driver private area */

    unsigned long	memPhysBase;		/* Physical address of FB */
    Bool		flipPixels;		/* swap default black/white */
    pointer		options;

    /* Allow screens to be enabled/disabled individually */
    Bool		vtSema;

    /*
     * Driver entry points.
     *
     */
    Bool		(*EnterVT)(int scrnIndex, int flags);
    void		(*LeaveVT)(int scrnIndex, int flags);
    Bool		(*SaveRestoreImage)(int scrnIndex,
					    SaveRestoreFlags what);
    int			(*SetDGAMode)(int scrnIndex, int num, 
					DGADevicePtr devRet);

} ScrnInfoRec, *ScrnInfoPtr;


typedef struct {
   Bool (*OpenFramebuffer)(
	ScrnInfoPtr pScrn, 
	char **name,
	unsigned char **mem, 
	int *size,
	int *offset,
        int *extra
   );
   void	(*CloseFramebuffer)(ScrnInfoPtr pScrn);
   Bool (*SetMode)(ScrnInfoPtr pScrn, DGAModePtr pMode);
   void (*SetViewport)(ScrnInfoPtr pScrn, int x, int y, int flags);
   int  (*GetViewport)(ScrnInfoPtr pScrn);
   void (*Sync)(ScrnInfoPtr);
   void (*FillRect)(
	ScrnInfoPtr pScrn, 
	int x, int y, int w, int h, 
	unsigned long color
   );
   void (*BlitRect)(
	ScrnInfoPtr pScrn, 
	int srcx, int srcy, 
	int w, int h, 
	int dstx, int dsty
   );
   void (*BlitTransRect)(
	ScrnInfoPtr pScrn, 
	int srcx, int srcy, 
	int w, int h, 
	int dstx, int dsty,
	unsigned long color
   );
} DGAFunctionRec, *DGAFunctionPtr;

/*
**	passed to xf86SetDepthBpp to indicate driver preferences.
*/
#define NoDepth24Support	0x00
#define Support24bppFb		0x01	/* 24bpp framebuffer supported */
#define Support32bppFb		0x02	/* 32bpp framebuffer supported */
#define SupportConvert24to32	0x04	/* Can convert 24bpp pixmap to 32bpp */
#define SupportConvert32to24	0x08	/* Can convert 32bpp pixmap to 24bpp */
#define PreferConvert24to32	0x10	/* prefer 24bpp pixmap to 32bpp conv */
#define PreferConvert32to24	0x20	/* prefer 32bpp pixmap to 24bpp conv */


/*
 * misc constants
 */

#define OVERLAY_8_32_DUALFB	0x00000001
#define OVERLAY_8_24_DUALFB	0x00000002
#define OVERLAY_8_16_DUALFB	0x00000004
#define OVERLAY_8_32_PLANAR	0x00000008

#endif /* _XF86STR_H */
