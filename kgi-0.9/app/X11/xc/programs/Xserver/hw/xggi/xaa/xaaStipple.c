/* $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xaaStipple.c,v 1.5 1999/08/14 10:50:14 dawes Exp $ */

#include "xaa.h"
#include "xaalocal.h"
#include "xaacexp.h"
#include "xf86.h"
#include "xf86_ansic.h"

static CARD32* StipplePowerOfTwo(CARD32*, CARD32*, int, int, int);
static CARD32* StipplePowerOfTwo_Inverted(CARD32*, CARD32*, int, int, int);
static CARD32* StippleUpTo32(CARD32*, CARD32*, int, int, int);
static CARD32* StippleUpTo32_Inverted(CARD32*, CARD32*, int, int, int);
static CARD32* StippleOver32(CARD32*, CARD32*, int, int, int);
static CARD32* StippleOver32_Inverted(CARD32*, CARD32*, int, int, int);

#define stipple_scanline_func EXPNAME(XAAStippleScanlineFunc)

StippleScanlineProcPtr stipple_scanline_func[6] = {
   StipplePowerOfTwo,
   StippleUpTo32,
   StippleOver32,
   StipplePowerOfTwo_Inverted,
   StippleUpTo32_Inverted,
   StippleOver32_Inverted
};


#ifdef FIXEDBASE
# define DEST(i)	*dest
# define RETURN(i)	return(dest)
#else
# define DEST(i)	dest[i]
# define RETURN(i)	return(dest + i)
#endif


#if defined(FIXEDBASE) && defined(MSBFIRST)

unsigned int XAAShiftMasks[32] = {
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F,
  0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,
  0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 
  0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,
  0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,
  0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF,
  0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF
};

#endif

void 
EXPNAME(XAAFillColorExpandRects)(
   ScrnInfoPtr pScrn,
   int fg, int bg, int rop,
   unsigned int planemask,
   int nBox,
   BoxPtr pBox,
   int xorg, int yorg,
   PixmapPtr pPix
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    CARD32 *base;
    Bool TwoPass = FALSE, FirstPass = TRUE;
    StippleScanlineProcPtr StippleFunc, FirstFunc, SecondFunc;
    int stipplewidth = pPix->drawable.width;
    int stippleheight = pPix->drawable.height;
    int srcwidth = pPix->devKind;
    int dwords, srcy, srcx, funcNo = 2, h;
    unsigned char *src = (unsigned char*)pPix->devPrivate.ptr;
    unsigned char *srcp;
    int flag;

    if(stipplewidth <= 32) {
	if(stipplewidth & (stipplewidth - 1))	
	  funcNo = 1;
	else	
	  funcNo = 0;
    } 
    StippleFunc = stipple_scanline_func[funcNo];

    if((bg == -1) || 
	!(infoRec->CPUToScreenColorExpandFillFlags & TRANSPARENCY_ONLY)) {
	/* one pass */
    } else if((rop == GXcopy) && infoRec->FillSolidRects) {
	/* one pass but we fill background rects first */
	(*infoRec->FillSolidRects)(pScrn, bg, rop, planemask, nBox, pBox);
	bg = -1;
    } else {
	/* gotta do two passes */
	TwoPass = TRUE;
	SecondFunc = stipple_scanline_func[funcNo];
	FirstFunc = stipple_scanline_func[funcNo + 3];
    }

    if(!TwoPass)
	(*infoRec->SetupForCPUToScreenColorExpandFill)(
					pScrn, fg, bg, rop, planemask);

    while(nBox--) {
	dwords = (pBox->x2 - pBox->x1 + 31) >> 5;

SECOND_PASS:
	if(TwoPass) {
	    (*infoRec->SetupForCPUToScreenColorExpandFill)(pScrn, 
			(FirstPass) ? bg : fg, -1, rop, planemask);
	    StippleFunc = (FirstPass) ? FirstFunc : SecondFunc;
	}

	h = pBox->y2 - pBox->y1;
	flag = (infoRec->CPUToScreenColorExpandFillFlags 
		& CPU_TRANSFER_PAD_QWORD) && ((dwords * h) & 0x01);

        (*infoRec->SubsequentCPUToScreenColorExpandFill)(
			pScrn, pBox->x1, pBox->y1,
 			pBox->x2 - pBox->x1, h, 0);

	base = (CARD32*)infoRec->ColorExpandBase;

	srcy = (pBox->y1 - yorg) % stippleheight;
	if(srcy < 0) srcy += stippleheight;
	srcx = (pBox->x1 - xorg) % stipplewidth;
	if(srcx < 0) srcx += stipplewidth;

	srcp = (srcwidth * srcy) + src;
	
#ifndef FIXEDBASE
	if((dwords * h) <= infoRec->ColorExpandRange) {
	   while(h--) {
		base = (*StippleFunc)(
			base, (CARD32*)srcp, srcx, stipplewidth, dwords);
		srcy++;
		srcp += srcwidth;
		if (srcy >= stippleheight) {
		   srcy = 0;
		   srcp = src;
		}
	   }
	} else
#endif
	   while(h--) {
		(*StippleFunc)(base, (CARD32*)srcp, srcx, stipplewidth, dwords);
		srcy++;
		srcp += srcwidth;
		if (srcy >= stippleheight) {
		   srcy = 0;
		   srcp = src;
		}
	   }
    
	  if (flag) {
	      base = (CARD32*)infoRec->ColorExpandBase;
	      base[0] = 0x00000000;
	  }

	if(TwoPass) {
	   if(FirstPass) {
		FirstPass = FALSE;
		goto SECOND_PASS;
	   } else FirstPass = TRUE;
	}

	pBox++;
     }

    if(infoRec->CPUToScreenColorExpandFillFlags & SYNC_AFTER_COLOR_EXPAND) 
	(*infoRec->Sync)(pScrn);
    else SET_SYNC_FLAG(infoRec);
}



void 
EXPNAME(XAAFillColorExpandSpans)(
   ScrnInfoPtr pScrn,
   int fg, int bg, int rop,
   unsigned int planemask,
   int n,
   DDXPointPtr ppt,
   int *pwidth,
   int fSorted,
   int xorg, int yorg,
   PixmapPtr pPix
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    CARD32 *base;
    Bool TwoPass = FALSE, FirstPass = TRUE;
    StippleScanlineProcPtr StippleFunc, FirstFunc, SecondFunc;
    int stipplewidth = pPix->drawable.width;
    int stippleheight = pPix->drawable.height;
    int dwords, srcy, srcx, funcNo = 2;
    unsigned char *srcp;

    if(stipplewidth <= 32) {
	if(stipplewidth & (stipplewidth - 1))	
	  funcNo = 1;
	else	
	  funcNo = 0;
    } 
    StippleFunc = stipple_scanline_func[funcNo];

    if((bg == -1) || 
	!(infoRec->CPUToScreenColorExpandFillFlags & TRANSPARENCY_ONLY)) {
	/* one pass */
    } else if((rop == GXcopy) && infoRec->FillSolidSpans) {
	/* one pass but we fill background rects first */
	(*infoRec->FillSolidSpans)(
		pScrn, bg, rop, planemask, n, ppt, pwidth, fSorted);
	bg = -1;
    } else {
	/* gotta do two passes */
	TwoPass = TRUE;
	SecondFunc = stipple_scanline_func[funcNo];
	FirstFunc = stipple_scanline_func[funcNo + 3];
    }

    if(!TwoPass)
	(*infoRec->SetupForCPUToScreenColorExpandFill)(
				pScrn, fg, bg, rop, planemask);

    while(n--) {
	dwords = (*pwidth + 31) >> 5;

	srcy = (ppt->y - yorg) % stippleheight;
	if(srcy < 0) srcy += stippleheight;
	srcx = (ppt->x - xorg) % stipplewidth;
	if(srcx < 0) srcx += stipplewidth;

	srcp = (pPix->devKind * srcy) + (unsigned char*)pPix->devPrivate.ptr;

SECOND_PASS:
	if(TwoPass) {
	    (*infoRec->SetupForCPUToScreenColorExpandFill)(pScrn, 
			(FirstPass) ? bg : fg, -1, rop, planemask);
	    StippleFunc = (FirstPass) ? FirstFunc : SecondFunc;
	}

        (*infoRec->SubsequentCPUToScreenColorExpandFill)(pScrn, ppt->x, ppt->y,
 			*pwidth, 1, 0);

	base = (CARD32*)infoRec->ColorExpandBase;

	(*StippleFunc)(base, (CARD32*)srcp, srcx, stipplewidth, dwords);
    
	if((infoRec->CPUToScreenColorExpandFillFlags & CPU_TRANSFER_PAD_QWORD) 
			&& (dwords & 0x01)) {
	    base = (CARD32*)infoRec->ColorExpandBase;
	    base[0] = 0x00000000;
    	}

	if(TwoPass) {
	   if(FirstPass) {
		FirstPass = FALSE;
		goto SECOND_PASS;
	   } else FirstPass = TRUE;
	}

	ppt++; pwidth++;
     }

    if(infoRec->CPUToScreenColorExpandFillFlags & SYNC_AFTER_COLOR_EXPAND) 
	(*infoRec->Sync)(pScrn);
    else SET_SYNC_FLAG(infoRec);
}


#ifndef FIXEDBASE

void 
EXPNAME(XAAFillScanlineColorExpandRects)(
   ScrnInfoPtr pScrn,
   int fg, int bg, int rop,
   unsigned int planemask,
   int nBox,
   BoxPtr pBox,
   int xorg, int yorg,
   PixmapPtr pPix
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    CARD32 *base;
    Bool TwoPass = FALSE, FirstPass = TRUE;
    StippleScanlineProcPtr StippleFunc, FirstFunc, SecondFunc;
    int stipplewidth = pPix->drawable.width;
    int stippleheight = pPix->drawable.height;
    int srcwidth = pPix->devKind;
    int dwords, srcy, srcx, funcNo = 2, bufferNo, h;
    unsigned char *src = pPix->devPrivate.ptr;
    unsigned char *srcp;

    if(stipplewidth <= 32) {
	if(stipplewidth & (stipplewidth - 1))	
	  funcNo = 1;
	else	
	  funcNo = 0;
    } 
    StippleFunc = stipple_scanline_func[funcNo];

    if((bg == -1) || 
      !(infoRec->ScanlineCPUToScreenColorExpandFillFlags & TRANSPARENCY_ONLY)) {
	/* one pass */
    } else if((rop == GXcopy) && infoRec->FillSolidRects) {
	/* one pass but we fill background rects first */
	(*infoRec->FillSolidRects)(pScrn, bg, rop, planemask, nBox, pBox);
	bg = -1;
    } else {
	/* gotta do two passes */
	TwoPass = TRUE;
	SecondFunc = stipple_scanline_func[funcNo];
	FirstFunc = stipple_scanline_func[funcNo + 3];
    }

    if(!TwoPass)
	(*infoRec->SetupForScanlineCPUToScreenColorExpandFill)(
				pScrn, fg, bg, rop, planemask);

    while(nBox--) {
	dwords = (pBox->x2 - pBox->x1 + 31) >> 5;
SECOND_PASS:
	if(TwoPass) {
	    (*infoRec->SetupForScanlineCPUToScreenColorExpandFill)(pScrn, 
			(FirstPass) ? bg : fg, -1, rop, planemask);
	    StippleFunc = (FirstPass) ? FirstFunc : SecondFunc;
	}

	h = pBox->y2 - pBox->y1;

        (*infoRec->SubsequentScanlineCPUToScreenColorExpandFill)(
		pScrn, pBox->x1, pBox->y1, pBox->x2 - pBox->x1, h, 0);

	bufferNo = 0;

	srcy = (pBox->y1 - yorg) % stippleheight;
	if(srcy < 0) srcy += stippleheight;
	srcx = (pBox->x1 - xorg) % stipplewidth;
	if(srcx < 0) srcx += stipplewidth;

	srcp = (srcwidth * srcy) + src;

	while(h--) {
   	    base = (CARD32*)infoRec->ScanlineColorExpandBuffers[bufferNo];
	    (*StippleFunc)(base, (CARD32*)srcp, srcx, stipplewidth, dwords);
	    (*infoRec->SubsequentColorExpandScanline)(pScrn, bufferNo++);
	    if(bufferNo >= infoRec->NumScanlineColorExpandBuffers)
		bufferNo = 0;
	    srcy++;
	    srcp += srcwidth;
	    if (srcy >= stippleheight) {
		srcy = 0;
		srcp = src;
	    }
	}
    
	if(TwoPass) {
	   if(FirstPass) {
		FirstPass = FALSE;
		goto SECOND_PASS;
	   } else FirstPass = TRUE;
	}

	pBox++;
     }

     SET_SYNC_FLAG(infoRec);
}

void 
EXPNAME(XAAFillScanlineColorExpandSpans)(
   ScrnInfoPtr pScrn,
   int fg, int bg, int rop,
   unsigned int planemask,
   int n,
   DDXPointPtr ppt,
   int *pwidth,
   int fSorted,
   int xorg, int yorg,
   PixmapPtr pPix
){
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCRNINFOPTR(pScrn);
    CARD32 *base;
    Bool TwoPass = FALSE, FirstPass = TRUE;
    StippleScanlineProcPtr StippleFunc, FirstFunc, SecondFunc;
    int stipplewidth = pPix->drawable.width;
    int stippleheight = pPix->drawable.height;
    int dwords, srcy, srcx, funcNo = 2;
    unsigned char *srcp;

    if(stipplewidth <= 32) {
	if(stipplewidth & (stipplewidth - 1))	
	  funcNo = 1;
	else	
	  funcNo = 0;
    } 
    StippleFunc = stipple_scanline_func[funcNo];

    if((bg == -1) || 
      !(infoRec->ScanlineCPUToScreenColorExpandFillFlags & TRANSPARENCY_ONLY)) {
	/* one pass */
    } else if((rop == GXcopy) && infoRec->FillSolidSpans) {
	/* one pass but we fill background rects first */
	(*infoRec->FillSolidSpans)(
		pScrn, bg, rop, planemask, n, ppt, pwidth, fSorted);
	bg = -1;
    } else {
	/* gotta do two passes */
	TwoPass = TRUE;
	SecondFunc = stipple_scanline_func[funcNo];
	FirstFunc = stipple_scanline_func[funcNo + 3];
    }

    if(!TwoPass)
	(*infoRec->SetupForScanlineCPUToScreenColorExpandFill)(
				pScrn, fg, bg, rop, planemask);


    while(n--) {
	dwords = (*pwidth + 31) >> 5;

	srcy = (ppt->y - yorg) % stippleheight;
	if(srcy < 0) srcy += stippleheight;
	srcx = (ppt->x - xorg) % stipplewidth;
	if(srcx < 0) srcx += stipplewidth;

	srcp = (pPix->devKind * srcy) + (unsigned char*)pPix->devPrivate.ptr;

SECOND_PASS:
	if(TwoPass) {
	    (*infoRec->SetupForScanlineCPUToScreenColorExpandFill)(pScrn, 
			(FirstPass) ? bg : fg, -1, rop, planemask);
	    StippleFunc = (FirstPass) ? FirstFunc : SecondFunc;
	}

        (*infoRec->SubsequentScanlineCPUToScreenColorExpandFill)(
				pScrn, ppt->x, ppt->y, *pwidth, 1, 0);

	base = (CARD32*)infoRec->ScanlineColorExpandBuffers[0];

	(*StippleFunc)(base, (CARD32*)srcp, srcx, stipplewidth, dwords);
	(*infoRec->SubsequentColorExpandScanline)(pScrn, 0);
    
	if(TwoPass) {
	   if(FirstPass) {
		FirstPass = FALSE;
		goto SECOND_PASS;
	   } else FirstPass = TRUE;
	}

	ppt++; pwidth++;
     }

     SET_SYNC_FLAG(infoRec);
}

#endif

static CARD32 *
StipplePowerOfTwo(
   CARD32* dest, CARD32* src, 
   int shift, int width, int dwords
){
    CARD32 pat = *src;
    if(width < 32) {
	pat &= XAAShiftMasks[width];
	while(width < 32) {
	    pat |= SHIFT_L(pat,width);
	    width <<= 1;
	}
    }
   
    if(shift)
	pat = SHIFT_R(pat,shift) | SHIFT_L(pat,32 - shift);

#ifdef MSBFIRST
    pat = XAAReverseBitOrder(pat);    
#endif

   while(dwords >= 4) {
	DEST(0) = pat;
	DEST(1) = pat;
	DEST(2) = pat;
	DEST(3) = pat;
	dwords -= 4;
#ifndef FIXEDBASE
	dest += 4;
#endif
   }
   
   if(!dwords) return dest;
   DEST(0) = pat;
   if(dwords == 1) RETURN(1);
   DEST(1) = pat;
   if(dwords == 2) RETURN(2);
   DEST(2) = pat;
   RETURN(3);
}

static CARD32 *
StipplePowerOfTwo_Inverted(
   CARD32* dest, CARD32* src, 
   int shift, int width, int dwords
){
    CARD32 pat = *src;
    if(width < 32) {
	pat &= XAAShiftMasks[width];
	while(width < 32) {
	    pat |= SHIFT_L(pat,width);
	    width <<= 1;
	}
    }
   
    if(shift)
	pat = SHIFT_R(pat,shift) | SHIFT_L(pat,32 - shift);

#ifdef MSBFIRST
    pat = XAAReverseBitOrder(pat);    
#endif

   pat = ~pat;

   while(dwords >= 4) {
	DEST(0) = pat;
	DEST(1) = pat;
	DEST(2) = pat;
	DEST(3) = pat;
	dwords -= 4;
#ifndef FIXEDBASE
	dest += 4;
#endif
   }
   
   if(!dwords) return dest;
   DEST(0) = pat;
   if(dwords == 1) RETURN(1);
   DEST(1) = pat;
   if(dwords == 2) RETURN(2);
   DEST(2) = pat;
   RETURN(3);
}


static CARD32 *
StippleUpTo32(
   CARD32* base, CARD32* src, 
   int shift, int width, int dwords
){
    int offset = shift;
    CARD32 pat = *src & XAAShiftMasks[width];

    while(width <= 15) {
	pat |= SHIFT_L(pat,width);
	width <<= 1;
    }
    pat |= SHIFT_L(pat,width);

    while(dwords--) {
	WRITE_BITS(SHIFT_R(pat,offset) | SHIFT_L(pat,width-offset));
	offset += 32;
	while(offset >= width) offset -= width;
    }
    return base;
}


static CARD32 *
StippleUpTo32_Inverted(
   CARD32* base, CARD32* src, 
   int shift, int width, int dwords
){
    CARD32 pat = *src & XAAShiftMasks[width];

    while(width <= 15) {
	pat |= SHIFT_L(pat,width);
	width <<= 1;
    }
    pat |= SHIFT_L(pat,width);

    while(dwords--) {
	WRITE_BITS(~(SHIFT_R(pat,shift) | SHIFT_L(pat,width-shift)));
	shift += 32;
	while(shift >= width) shift -= width;
    }
    return base;
}


static CARD32 *
StippleOver32(
   CARD32* base, CARD32* src, 
   int offset, int width, int dwords
){
   CARD32* srcp;
   int bitsleft, shift;   

   while(dwords--) {
	bitsleft = width - offset;
	srcp = (CARD32*)(src + (offset >> 5));
	shift = offset & 31;

	if(bitsleft >= 32) {
	    if(shift)
		WRITE_BITS(SHIFT_R(*srcp,shift) | 
			   SHIFT_L(srcp[1],32-shift));
	    else
		WRITE_BITS(*srcp);
	} else {
	    WRITE_BITS(SHIFT_L(*src,bitsleft) | 
		      (SHIFT_R(*srcp,shift) & XAAShiftMasks[bitsleft]));
	}
	offset += 32;
	while(offset >= width) offset -= width;
   }
   return base;
}


static CARD32 *
StippleOver32_Inverted(
   CARD32* base, CARD32* src, 
   int offset, int width, int dwords
){
   CARD32* srcp;
   int bitsleft, shift;   

   while(dwords--) {
	bitsleft = width - offset;
	srcp = (CARD32*)(src + (offset >> 5));
	shift = offset & 31;

	if(bitsleft >= 32) {
	    if(shift)
		WRITE_BITS(~(SHIFT_R(*srcp,shift) | 
			   SHIFT_L(srcp[1],32-shift)));
	    else
		WRITE_BITS(~(*srcp));
	} else {
	    WRITE_BITS(~(SHIFT_L(*src,bitsleft) | 
		      (SHIFT_R(*srcp,shift) & XAAShiftMasks[bitsleft])));
	}
	offset += 32;
	while(offset >= width) offset -= width;
   }
   return base;
}
