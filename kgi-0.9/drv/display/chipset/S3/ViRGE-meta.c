/* ----------------------------------------------------------------------------
**	S3 ViRGE chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-meta.c,v $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Jos_Hulzink	
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1 $"

#define	DEBUG_LEVEL	2

#include <kgi/module.h>

#define	__S3_ViRGE
#include "chipset/S3/ViRGE.h"
#include "chipset/S3/ViRGE-meta.h"
#include "chipset/S3/ViRGE-bind.h"

#define	VIRGE_ControlBase		((kgi_u8 *) virge_mem_control.base_virt)
#define	MEM_VGA			(VIRGE_ControlBase + VIRGE_VGABase)

#ifdef DEBUG_LEVEL
/*	Print verbose chipset configuration for debugging purposes
*/
static inline void virge_chipset_examine(virge_chipset_t *virge, 
	kgi_u32_t flags)
{
	kgi_u32_t mclk, memsize;
	kgi_u32_t foo;
	
	KRN_DEBUG(1, "virge_chipset_examine()");
	
	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "PCI Registers:");
	KRN_DEBUG(1, "--------------");
	KRN_DEBUG(1, "PCIBASE0 = %.8x", virge->pci.BaseAddr0);

	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "CRT Registers:");
	KRN_DEBUG(1, "--------------");
	KRN_DEBUG(1, "HTOTAL = %.2x", virge->crt.htotal);
	KRN_DEBUG(1, "HBLANKEND = %.2x", virge->crt.hblankend);
	KRN_DEBUG(1, "HRETRACESTART = %.2x", virge->crt.hretracestart);
	KRN_DEBUG(1, "HRETRACEEND = %.2x", virge->crt.hretraceend);
	KRN_DEBUG(1, "VTOTAL = %.2x", virge->crt.vtotal);
	KRN_DEBUG(1, "OVERFLOW = %.2x", virge->crt.overflow);
	KRN_DEBUG(1, "PRESETROWSCAN = %.2x", virge->crt.presetrowscan);
	KRN_DEBUG(1, "MAXSCANLINE = %.2x", virge->crt.maxscanline);
	KRN_DEBUG(1, "CURSORSTART = %.2x", virge->crt.cursorstart);
	KRN_DEBUG(1, "CURSOREND = %.2x", virge->crt.cursorend);
	KRN_DEBUG(1, "STARTADDRHIGH = %.2x", virge->crt.startaddrhigh);
	KRN_DEBUG(1, "STARTADDRLOW = %.2x", virge->crt.startaddrlow);
	KRN_DEBUG(1, "CURSORLOCATIONHIGH = %.2x", virge->crt.cursorlocationhigh);
	KRN_DEBUG(1, "CURSORLOCATIONLOW = %.2x", virge->crt.cursorlocationlow);
	KRN_DEBUG(1, "VRETRACESTART = %.2x", virge->crt.vretracestart);
	KRN_DEBUG(1, "VRETRACEEND = %.2x", virge->crt.vretraceend);
	KRN_DEBUG(1, "DISPLAYEND = %.2x", virge->crt.displayend);
	KRN_DEBUG(1, "OFFSET = %.2x", virge->crt.offset);
	KRN_DEBUG(1, "UNDERLINELOCATION = %.2x", virge->crt.underlinelocation);
	KRN_DEBUG(1, "VBLANKSTART = %.2x", virge->crt.vblankstart);
	KRN_DEBUG(1, "VBLANKEND = %.2x", virge->crt.vblankend);
	KRN_DEBUG(1, "MODECONTROL = %.2x", virge->crt.modecontrol);
	KRN_DEBUG(1, "LINECOMPARE = %.2x", virge->crt.linecompare);
/*
	KRN_DEBUG(1, "REPAINT0 = %.2x", virge->crt.repaint0);
	KRN_DEBUG(1, "REPAINT1 = %.2x", virge->crt.repaint1);
	KRN_DEBUG(1, "FIFOCONTROL = %.2x", virge->crt.fifocontrol);
	KRN_DEBUG(1, "FIFO = %.2x", virge->crt.fifo);

	KRN_DEBUG(1, "EXTRA = %.2x", virge->crt.extra);
	KRN_DEBUG(1, "PIXEL = %.2x", virge->crt.pixel);
	KRN_DEBUG(1, "HEXTRA = %.2x", virge->crt.hextra);
	KRN_DEBUG(1, "GRCURSOR0 = %.2x", virge->crt.grcursor0);
	KRN_DEBUG(1, "GRCURSOR1 = %.2x", virge->crt.grcursor1);
*/
	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "STREAMS Registers:");
	KRN_DEBUG(1, "--------------");
/*
	KRN_DEBUG(1, "GRCURSOR_START_POS = %.8x", virge->dac.grcursorstartpos);
	KRN_DEBUG(1, "NVPLL_COEFF = %.8x", virge->dac.nvpllcoeff);
	KRN_DEBUG(1, "MPLL_COEFF = %.8x", virge->dac.mpllcoeff);
	KRN_DEBUG(1, "VPLL_COEFF = %.8x", virge->dac.vpllcoeff);
	KRN_DEBUG(1, "PLL_COEFF_SELECT = %.8x", virge->dac.pllcoeffselect);
	KRN_DEBUG(1, "GENERAL_CONTROL = %.8x", virge->dac.generalcontrol);
*/
}
#endif	/* #if (DEBUG_LEVEL > 1) */

#if (DEBUG_LEVEL > 0)
/*	Symbolic subsystem names for post-reset initalization debug messages 
*/
static const kgi_ascii_t *virge_chipset_subsystem[16] =
{
	"CS",		"MC",		"GPFIFO",	"VC",
	"DAC",		"VS",		"VGA",		"R7",
	"GP",		"R9",		"RA",		"RB",
	"RC",		"RD",		"RE",		"RF"
};
#endif	/* #if (DEBUG_LEVEL > 0)	*/

static inline void virge_chipset_sync(virge_chipset_io_t *virge_io)
{
	kgi_u32_t count = 1;
	
	KRN_DEBUG(2, "virge_chipset_sync()");
	
	return;

#if 0
	do 
	{
		while (count--);
		count = VIRGE_CS_IN32(virge_io, VIRGE_CS_DMACount);

	} while (count);

	count = 1;
	
	do 
	{
		while (count++ < 255);
		count = VIRGE_CS_IN32(virge_io, VIRGE_CS_InFIFOSpace);

	} while (count);

	while (VIRGE_CS_IN32(virge_io, 0x068) & VIRGE_CS068_GraphicsProcessorActive);
#endif
}

/* Accelerator implementation */

typedef struct
{
	kgi_u32_t

#define	VIRGE_CORE_CONTEXT_GROUP00	0x007F8000
	Group00,
		StartXDom, dXDom, StartXSub, dXSub, StartY, dY, Count,

#define	VIRGE_CORE_CONTEXT_GROUP01	0x00238010
	Group01,
		RasterizerMode, YLimits, XLimits,

#define	VIRGE_CORE_CONTEXT_GROUP02	0x04008020
	Group02,
		PackedDataLimits,

#define	VIRGE_CORE_CONTEXT_GROUP03	0x021F8030
	Group03,
		ScissorMode, ScissorMinXY, ScissorMaxXY, ScreenSize,
		AreaStippleMode, WindowOrigin,

#define	VIRGE_CORE_CONTEXT_GROUP04	0x00FF8040
	Group04,
		AreaStipplePattern0, AreaStipplePattern1,
		AreaStipplePattern2, AreaStipplePattern3,
		AreaStipplePattern4, AreaStipplePattern5,
		AreaStipplePattern6, AreaStipplePattern7,

#define	VIRGE_CORE_CONTEXT_GROUP07	0x03FF8070
	Group07,
		TextureAddressMode, SStart, dSdx, dSdyDom, TStart, dTdx,
		dTdyDom, QStart, dQdx, dQdyDom,

#define	VIRGE_CORE_CONTEXT_GROUP0B	0x000780B0
	Group0B,
		TextureBaseAddress, TextureMapFormat, TextureDataFormat,

#define	VIRGE_CORE_CONTEXT_GROUP0C	0xC00180C0
	Group0C,
		Texel0, TextureReadMode, TexelLUTMode,

#define	VIRGE_CORE_CONTEXT_GROUP0D	0x7E7D80D0
	Group0D,
		TextureColorMode, FogMode, FogColor, FStart, dFdx, dFdyDom,
		KsStart, dKsdx, dKsdyDom, KdStart, Kddx, dKddyDom,

#define	VIRGE_CORE_CONTEXT_GROUP0F	0x33FF80F0
	Group0F,
		RStart, dRdx, dRdyDom, GStart, dGdx, dGdyDom, BStart, dBdx,
		dBdyDom, AStart, ColorDDAMode, ConstantColor,

#define	VIRGE_CORE_CONTEXT_GROUP10	0x007C8100
	Group10,
		AlphaBlendMode, DitherMode, FBSoftwareWriteMask, LogicalOpMode,
		FBWriteData,

#define	VIRGE_CORE_CONTEXT_GROUP11	0x43878110
	Group11,
		LBReadMode, LBReadFormat, LBSourceOffset, LBWindowBase,
		LBWriteMode, LBWriteFormat,

#define	VIRGE_CORE_CONTEXT_GROUP13	0x0FFF8130
	Group13,
		Window, StencilMode, StencilData, Stencil, DepthMode, Depth,
		ZStartU, ZStartL, dZdxU, dZdxL, dZdyDomU, dZdyDomL,

#define	VIRGE_CORE_CONTEXT_GROUP15	0x07C78150
	Group15,
		FBReadMode, FBSourceOffset, FBPixelOffset, FBWindowBase,
		FBWriteMode, FBHardwareWriteMask, FBBlockColor, FBReadPixel,

#define	VIRGE_CORE_CONTEXT_GROUP18	0x000F8180
	Group18,
		FilterMode, StatisticMode, MinRegion, MaxRegion,

#define	VIRGE_CORE_CONTEXT_GROUP1D	0xFFFF81D0
	Group1D,
		TexelLUT0, TexelLUT1, TexelLUT2, TexelLUT3,
		TexelLUT4, TexelLUT5, TexelLUT6, TexelLUT7,
		TexelLUT8, TexelLUT9, TexelLUTA, TexelLUTB,
		TexelLUTC, TexelLUTD, TexelLUTE, TexelLUTF,

#define	VIRGE_CORE_CONTEXT_GROUP1E	0x000781E0
	Group1E,
		YUVMode, ChromaUpperBound, ChromaLowerBound;

} virge_chipset_core_context_t;

typedef struct
{
	kgi_u32_t

#define	VIRGE_DELTA_CONTEXT_GROUP23	0x1FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z,

#define	VIRGE_DELTA_CONTEXT_GROUP24	0x1FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z,

#define	VIRGE_DELTA_CONTEXT_GROUP25	0x1FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z,

#define	VIRGE_DELTA_CONTEXT_GROUP26	0x80018260
	Group26,
		DeltaMode, BroadcastMask;

} virge_chipset_delta_context_t;

typedef struct
{
	kgi_u32_t

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP18	0x60008180
	Group18,
		FBBlockColorU, FBBlockColorL,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP1B	0x000781B0
	Group1B,
		FBSourceBase, FBSourceDelta, Config,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP1E	0x001881E0
	Group1E,
		AlphaMapUpperBound, AlphaMapLowerBound,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP23	0x5FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z, V0Float_color,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP24	0x5FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z, V1Float_color,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP25	0x5FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z, V2Float_color,

#define	VIRGE_PERMEDIA2_CONTEXT_GROUP26	0x00018260
	Group26,
		DeltaMode;

#warning handle texture LUT!

} virge_chipset_permedia2_context_t;


typedef struct
{
	kgi_accel_context_t kgi;
	kgi_aperture_t aperture;

	struct 
	{
		virge_chipset_core_context_t core;

		union 
		{
			virge_chipset_delta_context_t delta;
			virge_chipset_permedia2_context_t	p2;
		} ext;

	} state;

} virge_chipset_accel_context_t;

static void virge_chipset_accel_init(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "virge_chipset_accel_init()");
	
	return;
	
#if 0
	virge_chipset_t *virge = accel->meta;
	virge_chipset_io_t *virge_io = accel->meta_io;
	virge_chipset_accel_context_t *virge_ctx = ctx;
	kgi_size_t offset;

	/*	To be able to load ctx->state via DMA we precalculate the
	**	aperture info needed to have it at hand when needed.
	*/
	virge_ctx->aperture.size = sizeof(virge_ctx->state.core);
	if (virge->flags & VIRGE_CF_DELTA) {

		KRN_ASSERT(! (virge->flags & VIRGE_CF_PERMEDIA2));
		virge_ctx->aperture.size += sizeof(virge_ctx->state.ext.delta);
	}
	if (virge->flags & VIRGE_CF_PERMEDIA2) {

		KRN_ASSERT(! (virge->flags & VIRGE_CF_DELTA));
		virge_ctx->aperture.size += sizeof(virge_ctx->state.ext.p2);
	}
	offset = (mem_vaddr_t) &virge_ctx->state - (mem_vaddr_t) virge_ctx;
	virge_ctx->aperture.bus  = virge_ctx->kgi.aperture.bus  + offset;
	virge_ctx->aperture.phys = virge_ctx->kgi.aperture.phys + offset;
	virge_ctx->aperture.virt = virge_ctx->kgi.aperture.virt + offset;

	virge_ctx->state.core.Group00 = virge_CORE_CONTEXT_GROUP00;
	virge_ctx->state.core.Group01 = virge_CORE_CONTEXT_GROUP01;
	virge_ctx->state.core.Group02 = virge_CORE_CONTEXT_GROUP02;
	virge_ctx->state.core.Group03 = virge_CORE_CONTEXT_GROUP03;
	virge_ctx->state.core.Group04 = virge_CORE_CONTEXT_GROUP04;
	virge_ctx->state.core.Group07 = virge_CORE_CONTEXT_GROUP07;
	virge_ctx->state.core.Group0B = virge_CORE_CONTEXT_GROUP0B;
	virge_ctx->state.core.Group0C = virge_CORE_CONTEXT_GROUP0C;
	virge_ctx->state.core.Group0D = virge_CORE_CONTEXT_GROUP0D;
	virge_ctx->state.core.Group0F = virge_CORE_CONTEXT_GROUP0F;
	virge_ctx->state.core.Group10 = virge_CORE_CONTEXT_GROUP10;
	virge_ctx->state.core.Group11 = virge_CORE_CONTEXT_GROUP11,
	virge_ctx->state.core.Group13 = virge_CORE_CONTEXT_GROUP13;
	virge_ctx->state.core.Group15 = virge_CORE_CONTEXT_GROUP15;
	virge_ctx->state.core.Group18 = virge_CORE_CONTEXT_GROUP18;
	virge_ctx->state.core.Group1D = virge_CORE_CONTEXT_GROUP1D;
	virge_ctx->state.core.Group1E = virge_CORE_CONTEXT_GROUP1E;

	if (virge->flags & VIRGE_CF_DELTA) {

		virge_ctx->state.ext.delta.Group23 = virge_DELTA_CONTEXT_GROUP23;
		virge_ctx->state.ext.delta.Group24 = VIRGE_DELTA_CONTEXT_GROUP24;
		virge_ctx->state.ext.delta.Group25 = VIRGE_DELTA_CONTEXT_GROUP25;
		virge_ctx->state.ext.delta.Group26 = VIRGE_DELTA_CONTEXT_GROUP26;
	}

	if (virge->flags & VIRGE_CF_PERMEDIA2) {

		virge_ctx->state.ext.p2.Group18 = VIRGE_PERMEDIA2_CONTEXT_GROUP18;
		virge_ctx->state.ext.p2.Group1B = VIRGE_PERMEDIA2_CONTEXT_GROUP1B;
		virge_ctx->state.ext.p2.Group1E = VIRGE_PERMEDIA2_CONTEXT_GROUP1E;
		virge_ctx->state.ext.p2.Group23 = VIRGE_PERMEDIA2_CONTEXT_GROUP23;
		virge_ctx->state.ext.p2.Group24 = VIRGE_PERMEDIA2_CONTEXT_GROUP24;
		virge_ctx->state.ext.p2.Group25 = VIRGE_PERMEDIA2_CONTEXT_GROUP25;
		virge_ctx->state.ext.p2.Group26 = VIRGE_PERMEDIA2_CONTEXT_GROUP26;
	}
#endif
}

static void virge_chipset_accel_done(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "virge_chipset_accel_done()");
	
	if (ctx == accel->ctx) {

		accel->ctx = NULL;
	}
}

static inline void virge_chipset_accel_save(kgi_accel_t *accel)
{
	KRN_DEBUG(2, "virge_chipset_accel_save()");
	
	return;
	
#if 0
	
	virge_chipset_t *virge = accel->meta;
	virge_chipset_io_t *virge_io = accel->meta_io;
	virge_chipset_accel_context_t *virge_ctx = accel->ctx;
	mem_vaddr_t gpr = virge_io->control.base_virt + VIRGE_GPRegisterBase;

	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(virge_ctx);
	KRN_ASSERT(virge_io->control.base_virt);

	KRN_ASSERT(0 == VIRGE_CS_IN32(virge_io, VIRGE_CS_DMACount));

#define	VIRGE_SAVE(reg)	\
	virge_ctx->state.core.reg = mem_in32(gpr + (VIRGE_GP_##reg << 3))

	VIRGE_SAVE(StartXDom);
	VIRGE_SAVE(dXDom);
	VIRGE_SAVE(StartXSub);
	VIRGE_SAVE(dXSub);
	VIRGE_SAVE(StartY);
	VIRGE_SAVE(dY);
	VIRGE_SAVE(Count);

	VIRGE_SAVE(RasterizerMode);
	VIRGE_SAVE(YLimits);
	VIRGE_SAVE(XLimits);

	VIRGE_SAVE(PackedDataLimits);

	VIRGE_SAVE(ScissorMode);
	VIRGE_SAVE(ScissorMinXY);
	VIRGE_SAVE(ScissorMaxXY);
	VIRGE_SAVE(ScreenSize);
	VIRGE_SAVE(AreaStippleMode);
	VIRGE_SAVE(WindowOrigin);

	VIRGE_SAVE(AreaStipplePattern0);
	VIRGE_SAVE(AreaStipplePattern1);
	VIRGE_SAVE(AreaStipplePattern2);
	VIRGE_SAVE(AreaStipplePattern3);
	VIRGE_SAVE(AreaStipplePattern4);
	VIRGE_SAVE(AreaStipplePattern5);
	VIRGE_SAVE(AreaStipplePattern6);
	VIRGE_SAVE(AreaStipplePattern7);

	VIRGE_SAVE(TextureAddressMode);
	VIRGE_SAVE(SStart);
	VIRGE_SAVE(dSdx);
	VIRGE_SAVE(dSdyDom);
	VIRGE_SAVE(TStart);
	VIRGE_SAVE(dTdx);
	VIRGE_SAVE(dTdyDom);
	VIRGE_SAVE(QStart);
	VIRGE_SAVE(dQdx);
	VIRGE_SAVE(dQdyDom);

	VIRGE_SAVE(TextureBaseAddress);
	VIRGE_SAVE(TextureMapFormat);
	VIRGE_SAVE(TextureDataFormat);

	VIRGE_SAVE(Texel0);
	VIRGE_SAVE(TextureReadMode);
	VIRGE_SAVE(TexelLUTMode);

	VIRGE_SAVE(TextureColorMode);
	VIRGE_SAVE(FogMode);
	VIRGE_SAVE(FogColor);
	VIRGE_SAVE(FStart);
	VIRGE_SAVE(dFdx);
	VIRGE_SAVE(dFdyDom);
	VIRGE_SAVE(KsStart);
	VIRGE_SAVE(dKsdx);
	VIRGE_SAVE(dKsdyDom);
	VIRGE_SAVE(KdStart);
	VIRGE_SAVE(Kddx);
	VIRGE_SAVE(dKddyDom);

	VIRGE_SAVE(RStart);
	VIRGE_SAVE(dRdx);
	VIRGE_SAVE(dRdyDom);
	VIRGE_SAVE(GStart);
	VIRGE_SAVE(dGdx);
	VIRGE_SAVE(dGdyDom);
	VIRGE_SAVE(BStart);
	VIRGE_SAVE(dBdx);
	VIRGE_SAVE(dBdyDom);
	VIRGE_SAVE(AStart);
	VIRGE_SAVE(ColorDDAMode);
	VIRGE_SAVE(ConstantColor);

	VIRGE_SAVE(AlphaBlendMode);
	VIRGE_SAVE(DitherMode);
	VIRGE_SAVE(FBSoftwareWriteMask);
	VIRGE_SAVE(LogicalOpMode);

	VIRGE_SAVE(LBReadMode);
	VIRGE_SAVE(LBReadFormat);
	VIRGE_SAVE(LBSourceOffset);
	VIRGE_SAVE(LBWindowBase);
	VIRGE_SAVE(LBWriteMode);
	VIRGE_SAVE(LBWriteFormat);
	VIRGE_SAVE(FBWriteData);

	VIRGE_SAVE(Window);
	VIRGE_SAVE(StencilMode);
	VIRGE_SAVE(StencilData);
	VIRGE_SAVE(Stencil);
	VIRGE_SAVE(DepthMode);
	VIRGE_SAVE(Depth);
	VIRGE_SAVE(ZStartU);
	VIRGE_SAVE(ZStartL);
	VIRGE_SAVE(dZdxU);
	VIRGE_SAVE(dZdxL);
	VIRGE_SAVE(dZdyDomU);
	VIRGE_SAVE(dZdyDomL);

	VIRGE_SAVE(FBReadMode);
	VIRGE_SAVE(FBSourceOffset);
	VIRGE_SAVE(FBPixelOffset);
	VIRGE_SAVE(FBWindowBase);
	VIRGE_SAVE(FBWriteMode);
	VIRGE_SAVE(FBHardwareWriteMask);
	VIRGE_SAVE(FBBlockColor);
	VIRGE_SAVE(FBReadPixel);

	VIRGE_SAVE(FilterMode);
	VIRGE_SAVE(StatisticMode);
	VIRGE_SAVE(MinRegion);
	VIRGE_SAVE(MaxRegion);

	VIRGE_SAVE(TexelLUT0);
	VIRGE_SAVE(TexelLUT1);
	VIRGE_SAVE(TexelLUT2);
	VIRGE_SAVE(TexelLUT3);
	VIRGE_SAVE(TexelLUT4);
	VIRGE_SAVE(TexelLUT5);
	VIRGE_SAVE(TexelLUT6);
	VIRGE_SAVE(TexelLUT7);
	VIRGE_SAVE(TexelLUT8);
	VIRGE_SAVE(TexelLUT9);
	VIRGE_SAVE(TexelLUTA);
	VIRGE_SAVE(TexelLUTB);
	VIRGE_SAVE(TexelLUTC);
	VIRGE_SAVE(TexelLUTD);
	VIRGE_SAVE(TexelLUTE);
	VIRGE_SAVE(TexelLUTF);

	VIRGE_SAVE(YUVMode);
	VIRGE_SAVE(ChromaUpperBound);
	VIRGE_SAVE(ChromaLowerBound);

#undef	VIRGE_SAVE
	

#define	VIRGE_SAVE(reg)	\
	virge_ctx->state.ext.delta.reg = mem_in32(gpr + (DELTA_GC_##reg << 3))

	if (virge->flags & VIRGE_CF_DELTA) {

		VIRGE_SAVE(V0Float_s);
		VIRGE_SAVE(V0Float_t);
		VIRGE_SAVE(V0Float_q);
		VIRGE_SAVE(V0Float_Ks);
		VIRGE_SAVE(V0Float_Kd);
		VIRGE_SAVE(V0Float_red);
		VIRGE_SAVE(V0Float_green);
		VIRGE_SAVE(V0Float_blue);
		VIRGE_SAVE(V0Float_alpha);
		VIRGE_SAVE(V0Float_fog);
		VIRGE_SAVE(V0Float_x);
		VIRGE_SAVE(V0Float_y);
		VIRGE_SAVE(V0Float_z);

		VIRGE_SAVE(V1Float_s);
		VIRGE_SAVE(V1Float_t);
		VIRGE_SAVE(V1Float_q);
		VIRGE_SAVE(V1Float_Ks);
		VIRGE_SAVE(V1Float_Kd);
		VIRGE_SAVE(V1Float_red);
		VIRGE_SAVE(V1Float_green);
		VIRGE_SAVE(V1Float_blue);
		VIRGE_SAVE(V1Float_alpha);
		VIRGE_SAVE(V1Float_fog);
		VIRGE_SAVE(V1Float_x);
		VIRGE_SAVE(V1Float_y);
		VIRGE_SAVE(V1Float_z);

		VIRGE_SAVE(V2Float_s);
		VIRGE_SAVE(V2Float_t);
		VIRGE_SAVE(V2Float_q);
		VIRGE_SAVE(V2Float_Ks);
		VIRGE_SAVE(V2Float_Kd);
		VIRGE_SAVE(V2Float_red);
		VIRGE_SAVE(V2Float_green);
		VIRGE_SAVE(V2Float_blue);
		VIRGE_SAVE(V2Float_alpha);
		VIRGE_SAVE(V2Float_fog);
		VIRGE_SAVE(V2Float_x);
		VIRGE_SAVE(V2Float_y);
		VIRGE_SAVE(V2Float_z);

		VIRGE_SAVE(DeltaMode);
		VIRGE_SAVE(BroadcastMask);
	}

#undef	VIRGE_SAVE
#define	VIRGE_SAVE(reg)	\
	virge_ctx->state.ext.p2.reg = mem_in32(gpr + (VIRGE_GP_##reg << 3))

	if (virge->flags & VIRGE_CF_PERMEDIA2) {

		VIRGE_SAVE(FBBlockColorU);
		VIRGE_SAVE(FBBlockColorL);

		VIRGE_SAVE(FBSourceBase);
		VIRGE_SAVE(FBSourceDelta);
		VIRGE_SAVE(Config);

		VIRGE_SAVE(AlphaMapUpperBound);
		VIRGE_SAVE(AlphaMapLowerBound);

		VIRGE_SAVE(V0Float_s);
		VIRGE_SAVE(V0Float_t);
		VIRGE_SAVE(V0Float_q);
		VIRGE_SAVE(V0Float_Ks);
		VIRGE_SAVE(V0Float_Kd);
		VIRGE_SAVE(V0Float_red);
		VIRGE_SAVE(V0Float_green);
		VIRGE_SAVE(V0Float_blue);
		VIRGE_SAVE(V0Float_alpha);
		VIRGE_SAVE(V0Float_fog);
		VIRGE_SAVE(V0Float_x);
		VIRGE_SAVE(V0Float_y);
		VIRGE_SAVE(V0Float_z);
		VIRGE_SAVE(V0Float_color);

		VIRGE_SAVE(V1Float_s);
		VIRGE_SAVE(V1Float_t);
		VIRGE_SAVE(V1Float_q);
		VIRGE_SAVE(V1Float_Ks);
		VIRGE_SAVE(V1Float_Kd);
		VIRGE_SAVE(V1Float_red);
		VIRGE_SAVE(V1Float_green);
		VIRGE_SAVE(V1Float_blue);
		VIRGE_SAVE(V1Float_alpha);
		VIRGE_SAVE(V1Float_fog);
		VIRGE_SAVE(V1Float_x);
		VIRGE_SAVE(V1Float_y);
		VIRGE_SAVE(V1Float_z);
		VIRGE_SAVE(V1Float_color);

		VIRGE_SAVE(V2Float_s);
		VIRGE_SAVE(V2Float_t);
		VIRGE_SAVE(V2Float_q);
		VIRGE_SAVE(V2Float_Ks);
		VIRGE_SAVE(V2Float_Kd);
		VIRGE_SAVE(V2Float_red);
		VIRGE_SAVE(V2Float_green);
		VIRGE_SAVE(V2Float_blue);
		VIRGE_SAVE(V2Float_alpha);
		VIRGE_SAVE(V2Float_fog);
		VIRGE_SAVE(V2Float_x);
		VIRGE_SAVE(V2Float_y);
		VIRGE_SAVE(V2Float_z);
		VIRGE_SAVE(V2Float_color);

		VIRGE_SAVE(DeltaMode);
	}
#undef	VIRGE_SAVE
	
#endif
}

static inline void virge_chipset_accel_restore(kgi_accel_t *accel)
{
	virge_chipset_io_t *virge_io = accel->meta_io;
	virge_chipset_accel_context_t *virge_ctx = accel->ctx;

	KRN_DEBUG(2, "virge_chipset_accel_restore()");
#if 0	
	KRN_ASSERT(0 == VIRGE_CS_IN32(virge_io, VIRGE_CS_DMACount));

#warning flush cache-lines of the context buffer!

	VIRGE_CS_OUT32(virge_io, virge_ctx->aperture.bus, VIRGE_CS_DMAAddress);
	VIRGE_CS_OUT32(virge_io, virge_ctx->aperture.size >> 2, VIRGE_CS_DMACount);
#endif
}

static void virge_chipset_accel_schedule(kgi_accel_t *accel)
{
#if 0
#warning this must not be interrupted!
	virge_chipset_io_t *virge_io = accel->meta_io;
	kgi_accel_buffer_t *buffer = accel->exec_queue;

	KRN_DEBUG(2, "virge_chipset_accel_schedule()");
	
	KRN_ASSERT(buffer);

	switch (buffer->exec_state) 
	{
	case KGI_AS_EXEC:
		/*	Execution of the current buffer finished, so we 
		**	mark it KGI_AS_IDLE and remove it from the queue.
		*/
		accel->exec_queue = buffer->next;
		buffer->next = NULL;
		buffer->exec_state = KGI_AS_IDLE;
#warning wakeup buffer->executed !

		if (NULL == accel->exec_queue) 
		{
			/*	no further buffers queued, thus we are done */
#warning wakeup virge_accel->idle
			return;
		}

		buffer = accel->exec_queue;
		KRN_ASSERT(KGI_AS_WAIT == buffer->exec_state);
		/* Fall through */
	case KGI_AS_WAIT:
		/*	If necessary we initiate a GP context switch and
		**	re-enter here when the new context is loaded.
		**	Otherwise we just initiate the buffer transfer.
		*/
		if (accel->ctx != buffer->exec_ctx) 
		{
			if (accel->ctx) 
			{
				virge_chipset_accel_save(accel);
			}
			
			accel->ctx = buffer->exec_ctx;
			virge_chipset_accel_restore(accel);
			
			return;
		}

		KRN_ASSERT(0 == VIRGE_CS_IN32(virge_io, VIRGE_CS_DMACount));

		buffer->exec_state = KGI_AS_EXEC;
		
		VIRGE_CS_OUT32(virge_io, buffer->aperture.bus, VIRGE_CS_DMAAddress);
		VIRGE_CS_OUT32(virge_io, buffer->exec_size >> 2, VIRGE_CS_DMACount);

		return;

	default:
		KRN_ERROR("PERMEDIA: invalid state %i for queued buffer", buffer->exec_state);
		KRN_INTERNAL_ERROR;
		return;
	}
#endif
}

static void virge_chipset_accel_exec(kgi_accel_t *accel, 
	kgi_accel_buffer_t *buffer)
{
#warning check/validate validate data stream!!!
#warning this must not be interrupted!

	KRN_DEBUG(2, "virge_chipset_accel_exec()");
	
	KRN_ASSERT(KGI_AS_FILL == buffer->exec_state);

	buffer->exec_state = KGI_AS_WAIT;

	if (accel->exec_queue) {

		kgi_accel_buffer_t *queued = accel->exec_queue;

		while (queued->next && 
			(queued->next->exec_pri >= buffer->exec_pri)) {

			queued = queued->next;
		}

		buffer->next = queued->next;
		queued->next = buffer;
		
		return;
	}

	buffer->next = NULL;
	accel->exec_queue = buffer;

	virge_chipset_accel_schedule(accel);
}

/*	IRQ and error handlers
*/

static inline void virge_chipset_error_handler(virge_chipset_t *virge,
	virge_chipset_io_t *virge_io, irq_system_t *system)
{
	KRN_DEBUG(2, "virge_chipset_error_handler()");
	
#if 0
	kgi_u32_t handled = 0;
	kgi_u32_t flags = virge_CS_IN32(virge_io, virge_CS_ErrorFlags);

#define	VIRGE_ERROR(err, msg)						\
	if (flags & VIRGE_CS038_##err##Error) {				\
									\
		handled |= VIRGE_CS038_##err##Error;			\
		KRN_TRACE(0, virge->error.err++);			\
		KRN_ERROR(msg " (pcidev %.8x)", VIRGE_PCIDEV(virge_io));\
	}

	VIRGE_ERROR(InFIFO, "write to full input FIFO");
	VIRGE_ERROR(OutFIFO, "read from empty output FIFO");
	VIRGE_ERROR(Message, "incorrect FIFO/GC access");
	VIRGE_ERROR(DMA, "input FIFO write during DMA transfer");
	VIRGE_ERROR(VideoFifoUnderflow, "video FIFO underflow");
	VIRGE_ERROR(VSBUnderflow, "Stream B: FIFO underflow");
	VIRGE_ERROR(VSAUnderflow, "Stream A: FIFO underflow");
	VIRGE_ERROR(Master, "PCI bus-mastering error");
	VIRGE_ERROR(OutDMA, "output FIFO read during DMA transfer");
	VIRGE_ERROR(InDMAOverwrite, "InDMACount overwritten");
	VIRGE_ERROR(OutDMAOverwrite, "OutDMACount overwritten");
	VIRGE_ERROR(VSAInvalidInterlace, "Stream A: invalid interlace");
	VIRGE_ERROR(VSBInvalidInterlace, "Stream B: invalid interlace");

#undef	VIRGE_ERROR

	if (flags & ~handled) {

		KRN_TRACE(0, virge->error.unknown++);
		KRN_ERROR("unknown virge error(s): %.8x", flags & ~handled);
	}

	if (! handled) { 

		KRN_TRACE(0, virge->error.no_reason++);
		KRN_ERROR("virge error interrupt, but no errors indicated.");
	}

	VIRGE_CS_OUT32(virge_io, handled, VIRGE_CS_ErrorFlags);
#endif
}

kgi_error_t virge_chipset_irq_handler(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, irq_system_t *system)
{
#if 0
	kgi_u32_t handled = 0;
	kgi_u32_t flags = VIRGE_CS_IN32(virge_io, virge_CS_IntFlags);

	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);

	if (flags & VIRGE_CS010_DMAFlag) {

		handled |= VIRGE_CS010_DMAFlag;
		KRN_TRACE(0, virge->interrupt.DMA++);
		KRN_DEBUG(1, "DMA interrupt");
		
		if (virge->mode) {

			virge_chipset_accel_schedule(&(virge->mode->virge.gp_accel));
		}
	}

	if (flags & VIRGE_CS010_SyncFlag) {

		handled |= VIRGE_CS010_SyncFlag;
		KRN_TRACE(0, virge->interrupt.Sync++);
		KRN_DEBUG(1, "Sync interrupt (pcidev %.8x)", VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_ErrorFlag) {

		handled |= VIRGE_CS010_ErrorFlag;
		KRN_TRACE(0, virge->interrupt.Error++);
		virge_chipset_error_handler(virge, virge_io, system);
	}

	if (flags & VIRGE_CS010_VRetraceFlag) {

		handled |= VIRGE_CS010_VRetraceFlag;
		KRN_TRACE(0, virge->interrupt.VRetrace++);
	}

	if (flags & VIRGE_CS010_ScanlineFlag) {

		handled |= VIRGE_CS010_ScanlineFlag;
		KRN_TRACE(0, virge->interrupt.Scanline++);
	}

	if (flags & VIRGE_CS010_TextureInvalidFlag) {

		handled |= VIRGE_CS010_TextureInvalidFlag;
		KRN_TRACE(0, virge->interrupt.TextureInvalid++);
		KRN_DEBUG(1, "texture invalid interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_BypassDMAIntFlag) {

		handled |= VIRGE_CS010_BypassDMAIntFlag;
		KRN_TRACE(0, virge->interrupt.BypassDMA++);
		KRN_DEBUG(1, "bypass DMA interrupt (pcidev %.8x)",
			virge_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_VSBFlag) {

		handled |= VIRGE_CS010_VSBFlag;
		KRN_TRACE(0, virge->interrupt.VSB++);
		KRN_DEBUG(1, "video stream B interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_VSAFlag) {

		handled |= VIRGE_CS010_VSAFlag;
		KRN_TRACE(0, virge->interrupt.VSA++);
		KRN_DEBUG(1, "video stream A interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_VideoStreamSerialFlag) {

		handled |= VIRGE_CS010_VideoStreamSerialFlag;
		KRN_TRACE(0, virge->interrupt.VideoStreamSerial++);
		KRN_DEBUG(1, "video stream serial interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_VideoDDCFlag) {

		handled |= VIRGE_CS010_VideoDDCFlag;
		KRN_TRACE(0, virge->interrupt.VideoDDC++);
		KRN_DEBUG(1, "video DDC interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & VIRGE_CS010_VideoStreamExternalFlag) {

		handled |= VIRGE_CS010_VideoStreamExternalFlag;
		KRN_TRACE(0, virge->interrupt.VideoStreamExternal++);
		KRN_DEBUG(1, "video stream external interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}

	if (flags & ~handled) {

		KRN_TRACE(0, virge->interrupt.not_handled++);
		KRN_ERROR("virge: unhandled interrupt flags %.8x (pcidev %.8x)",
			flags & ~handled, VIRGE_PCIDEV(virge_io));
	}

	if (!flags) {

		KRN_TRACE(0, virge->interrupt.no_reason++);
		KRN_ERROR("virge: interrupt but no reason indicated.");
	}

	VIRGE_CS_OUT32(virge_io, handled, VIRGE_CS_IntFlags);

#endif
	return KGI_EOK;
}

#warning If you need those, extend chipset_io_t appropriately!
/*	no extern declarations outside the header files allowed!
extern kgi_u8_t virge_chipset_vga_crt_in8(virge_chipset_io_t *io, kgi_u_t reg);
extern kgi_u32_t virge_chipset_ctl_in32(virge_chipset_io_t *mem, kgi_u_t reg);
*/


kgi_error_t virge_chipset_init(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = VIRGE_PCIDEV(virge_io);

	KRN_DEBUG(2, "virge_chipset_init()");
	
	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "Initializing %s %s", 
		virge->chipset.vendor, virge->chipset.model);

#define	PCICFG_SET_BASE(value, reg)		\
	pcicfg_out32(0xFFFFFFFF, reg);	\
	pcicfg_in32(reg);		\
	pcicfg_out32((value), reg)

	PCICFG_SET_BASE(virge_io->aperture.base_io,
		pcidev + PCI_BASE_ADDRESS_0);
/*	PCICFG_SET_BASE(virge_io->framebuffer.base_io,
**		pcidev + PCI_BASE_ADDRESS_1);
*/	
#undef	PCICFG_SET_BASE

	KRN_DEBUG(1, "PCI (re-)configuration done");

	if (virge->pci.Command & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) {

		KRN_DEBUG(2, "Chipset initialized, reading configuration");
		
		virge->flags |= VIRGE_CF_RESTORE_INITIAL;
		
#if 0
		            virge->crt.htotal = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_TOTAL);
		         virge->crt.hblankend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_BLANK_END);
		     virge->crt.hretracestart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_RETRACE_START);
		       virge->crt.hretraceend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_RETRACE_END);
		            virge->crt.vtotal = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_TOTAL);
		          virge->crt.overflow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_OVERFLOW);
		     virge->crt.presetrowscan = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_PRESET_ROW_SCAN);
		       virge->crt.maxscanline = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_MAX_SCAN_LINE);
		       virge->crt.cursorstart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_START);
		         virge->crt.cursorend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_END);
		     virge->crt.startaddrhigh = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_START_ADDR_HIGH);
		      virge->crt.startaddrlow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_START_ADDR_LOW);
		virge->crt.cursorlocationhigh = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_LOCATION_HIGH);
		 virge->crt.cursorlocationlow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_LOCATION_LOW);
		     virge->crt.vretracestart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_RETRACE_START);
		       virge->crt.vretraceend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_RETRACE_END);
		        virge->crt.displayend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_DISPLAY_END);
		            virge->crt.offset = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_OFFSET);
		 virge->crt.underlinelocation = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_UNDERLINE_LOCATION);
		       virge->crt.vblankstart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_BLANK_START);
		         virge->crt.vblankend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_BLANK_END);
		       virge->crt.modecontrol = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_MODE_CONTROL);
		       virge->crt.linecompare = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_LINE_COMPARE);
		          virge->crt.repaint0 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_REPAINT0);
		          virge->crt.repaint1 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_REPAINT1);
		       virge->crt.fifocontrol = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_FIFO_CONTROL);
		              virge->crt.fifo = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_FIFO);
		             virge->crt.extra = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_EXTRA);
		             virge->crt.pixel = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_PIXEL);
		            virge->crt.hextra = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_EXTRA);
		         virge->crt.grcursor0 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_GRCURSOR0);
		         virge->crt.grcursor1 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_GRCURSOR1);

		  virge->dac.grcursorstartpos = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_GRCURSOR_START_POS);
		        virge->dac.nvpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_NVPLL_COEFF);
		         virge->dac.mpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_MPLL_COEFF);
		         virge->dac.vpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_VPLL_COEFF);
		    virge->dac.pllcoeffselect = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_PLL_COEFF_SELECT);
		    virge->dac.generalcontrol = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_GENERAL_CONTROL);
#endif
		vga_text_chipset_init(&(virge->vga), &(virge_io->vga), options);

	} else {

		kgi_u_t cnt = 0;

		KRN_DEBUG(2, "Chipset not initialized, resetting");

#if 0
		            virge->crt.htotal = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_TOTAL);
		         virge->crt.hblankend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_BLANK_END);
		     virge->crt.hretracestart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_RETRACE_START);
		       virge->crt.hretraceend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_RETRACE_END);
		            virge->crt.vtotal = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_TOTAL);
		          virge->crt.overflow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_OVERFLOW);
		     virge->crt.presetrowscan = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_PRESET_ROW_SCAN);
		       virge->crt.maxscanline = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_MAX_SCAN_LINE);
		       virge->crt.cursorstart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_START);
		         virge->crt.cursorend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_END);
		     virge->crt.startaddrhigh = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_START_ADDR_HIGH);
		      virge->crt.startaddrlow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_START_ADDR_LOW);
		virge->crt.cursorlocationhigh = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_LOCATION_HIGH);
		 virge->crt.cursorlocationlow = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_CURSOR_LOCATION_LOW);
		     virge->crt.vretracestart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_RETRACE_START);
		       virge->crt.vretraceend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_RETRACE_END);
		        virge->crt.displayend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_DISPLAY_END);
		            virge->crt.offset = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_OFFSET);
		 virge->crt.underlinelocation = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_UNDERLINE_LOCATION);
		       virge->crt.vblankstart = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_BLANK_START);
		         virge->crt.vblankend = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_VERT_BLANK_END);
		       virge->crt.modecontrol = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_MODE_CONTROL);
		       virge->crt.linecompare = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_LINE_COMPARE);
		          virge->crt.repaint0 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_REPAINT0);
		          virge->crt.repaint1 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_REPAINT1);
		       virge->crt.fifocontrol = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_FIFO_CONTROL);
		              virge->crt.fifo = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_FIFO);
		             virge->crt.extra = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_EXTRA);
		             virge->crt.pixel = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_PIXEL);
		            virge->crt.hextra = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_HORIZ_EXTRA);
		         virge->crt.grcursor0 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_GRCURSOR0);
		         virge->crt.grcursor1 = virge_chipset_vga_crt_in8(virge_io, NV_PCRTC_GRCURSOR1);

		  virge->dac.grcursorstartpos = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_GRCURSOR_START_POS);
		        virge->dac.nvpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_NVPLL_COEFF);
		         virge->dac.mpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_MPLL_COEFF);
		         virge->dac.vpllcoeff = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_VPLL_COEFF);
		    virge->dac.pllcoeffselect = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_PLL_COEFF_SELECT);
		    virge->dac.generalcontrol = virge_chipset_ctl_in32(virge_io, NV_PRAMDAC_GENERAL_CONTROL);
#endif
		
		vga_text_chipset_init(&(virge->vga), &(virge_io->vga), options);
	}

	KRN_DEBUG(2, "Chipset enabled");

	/* Detect RAM size */
#if 0	
	virge->fb.boot0 = virge_chipset_ctl_in32(virge_io, NV_PFB_BOOT_0);
	
	switch(virge->fb.boot0 & 0x00000003) {

	case NV_PFB_BOOT_0_RAM_AMOUNT_4MB:
		KRN_DEBUG(1, "4MB video memory detected");
		virge->chipset.memory = 4 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_8MB:
		KRN_DEBUG(1, "8MB video memory detected");
		virge->chipset.memory = 8 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_16MB:
		KRN_DEBUG(1, "4MB video memory detected");
		virge->chipset.memory = 16 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_32MB:
		KRN_DEBUG(1, "32MB video memory detected");
		virge->chipset.memory = 32 MB;
		break;
	}
	
	switch(virge->fb.boot0 & 0x00000030) {

	case NV_PMC_BOOT_0_MAJOR_REVISION_A:
		KRN_DEBUG(1, "Major revision A detected");
		/* video bandwidth = 800000 K/sec */
		break;
	
	default:
		/* video bandwidth = 1000000 K/sec */
		break;
	}
#endif
	
#if 0
	chip->CrystalFreqKHz   = (chip->PEXTDEV[0x00000000 / 4] & 0x00000040) ? 14318 : 13500;
	chip->CURSOR           = &(chip->PRAMIN[0x00010000 / 4 - 0x0800 / 4]);
	chip->CURSORPOS        = &(chip->PRAMDAC[0x0300 / 4]);
	chip->VBLANKENABLE     = &(chip->PCRTC[0x0140 / 4]);
	chip->VBLANK           = &(chip->PCRTC[0x0100 / 4]);
	chip->VBlankBit        = 0x00000001;
	chip->MaxVClockFreqKHz = 250000;
	chip->LockUnlockIO     = 0x3D4;
	chip->LockUnlockIndex  = 0x1F;
#endif
	
	KRN_TRACE(2, virge_chipset_examine(virge, 1));

	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) {

//		VIRGE_CS_OUT32(virge_io, VIRGE_CS010_SyncFlag | VIRGE_CS010_ErrorFlag | VIRGE_CS010_VRetraceFlag, 0x010);
//		VIRGE_CS_OUT32(virge_io, VIRGE_CS008_SyncIntEnable | VIRGE_CS008_ErrorIntEnable | VIRGE_CS008_VRetraceIntEnable, 0x008);
	}

	return KGI_EOK;
}

void virge_chipset_done(virge_chipset_t *virge, virge_chipset_io_t *virge_io, 
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = VIRGE_PCIDEV(virge_io);

	KRN_DEBUG(2, "virge_chipset_done()");
	
	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) {

//		VIRGE_CS_OUT32(virge_io, VIRGE_CS008_DisableInterrupts, 0x008);
	}

	if (virge->flags & VIRGE_CF_RESTORE_INITIAL) {

		KRN_DEBUG(2, "Restoring initial chipset state");
		vga_text_chipset_done((&virge->vga), &(virge_io->vga), options);
	}
}

kgi_error_t virge_chipset_mode_check(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = virge_mode->virge.kgim.crt;
	kgi_u_t shift, bpf, bpc, bpp, pgm, width, lclk, pp[3];
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t mul, div, bpd;

	KRN_DEBUG(2, "virge_chipset_mode_check()");
	
	if (images != 1) {

		KRN_DEBUG(2, "%i images not supported.", images);
		return -E(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		return vga_text_chipset_mode_check(&virge->vga, &virge_io->vga, 
			&virge_mode->vga, cmd, img, images);
	}
	
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) {

		KRN_DEBUG(1, "Image flags %.8x not supported", img[0].flags);
		return -E(CHIPSET, INVAL);
	}

	/*	Check if common attributes are supported
	*/
	switch (img[0].cam) {

	case 0:
		KRN_DEBUG(2, "img[0].cam = 0");
		break;

	case KGI_AM_STENCIL | KGI_AM_Z:
		
		if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) {

			KRN_DEBUG(2, "S%iZ%i local buffer not supported",
				img[0].bpca[0], img[0].bpca[1]);
			return -E(CHIPSET, INVAL);
		}
		break;

	case KGI_AM_Z:
		if (16 != img[0].bpca[1]) {

			KRN_DEBUG(2, "Z%i local buffer not supported",
				img[0].bpca[0]);
			return -E(CHIPSET, INVAL);
		}
		
	default:
		KRN_DEBUG(2, "Common attributes %.8x not supported",
			img[0].cam);
		return -E(CHIPSET, INVAL);
	}

	/*	total bits per dot
	*/
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO)
		? (bpc + bpf * img[0].frames * 2) 
		: (bpc + bpf*img[0].frames);

	shift = 0;
	
	switch (bpd) {

	case  1:	shift++;	/* fall through	*/
	case  2:	shift++;	/* fall through */
	case  4:	shift++;	/* fall through	*/
	case  8:	shift++;	/* fall through	*/
	case 16:	shift++;	/* fall through	*/
	case 32:
		pgm = 1;
		pgm = (pgm << shift) - 1;
		break;

	default:	
		KRN_DEBUG(0, "%i bpd not supported", bpd);
		return -E(CHIPSET, FAILED);
	}

	lclk = (cmd == KGI_TC_PROPOSE) 
		? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE");
		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/*	If size.x or size.y are zero, default to 640x400
		*/
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			KRN_DEBUG(2, "Defaulting to 640x480");
			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			KRN_DEBUG(2, "Defaulting to size,x, size.y");
			
			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) {

			KRN_DEBUG(2, "virt.x = 0");
			
			img[0].virt.x = (8 * virge->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > virge->chipset.maxdots.x) {

				KRN_DEBUG(2, "virt.x > maxdots.x");
				img[0].virt.x = virge->chipset.maxdots.x;
			}
		}
		
		if (0 == img[0].virt.y) {

			KRN_DEBUG(2, "virt.y = 0");
			img[0].virt.y = (8 * virge->chipset.memory) / 
				(img[0].virt.x * bpp);
		}

		/*	Are we beyond the limits of the H/W?
		*/
		if ((img[0].size.x >= virge->chipset.maxdots.x) ||
			(img[0].virt.x >= virge->chipset.maxdots.x)) {

			KRN_DEBUG(2, "%i (%i) horizontal pixels are too many",
				img[0].size.x, img[0].virt.x);
			return -E(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= virge->chipset.maxdots.y) ||
			(img[0].virt.y >= virge->chipset.maxdots.y)) {

			KRN_DEBUG(2, "%i (%i) vertical pixels are too many",
				img[0].size.y, img[0].virt.y);
			return -E(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
			(8 * virge->chipset.memory)) {

			KRN_DEBUG(2, "not enough memory for "
				"(%ipf*%if + %ipc)@%ix%i", bpf, img[0].frames,
				bpc, img[0].virt.x, img[0].virt.y);
			return -E(CHIPSET,NOMEM);
		}

		/*	Take screen visible width up to next 32/64-bit word 
		*/
#if 0
		if (img[0].size.x & pgm) {

			img[0].size.x &= ~pgm;
			img[0].size.x += pgm + 1;
		}
#endif

		/*	Set CRT visible fields 
		*/
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}
		
		KRN_DEBUG(2, "dots.x = %d, dots.y = %d", 
			dpm->dots.x, dpm->dots.y);
		
		return KGI_EOK;

	case KGI_TC_LOWER:
	case KGI_TC_RAISE:
		KRN_DEBUG(3, (cmd == KGI_TC_LOWER) 
			? "KGI_TC_LOWER" : "KGI_TC_RAISE");

		/*	Adjust lclk and rclk. Use 64 bit bus on P2, 32 on P1
		*/
		dpm->lclk.mul = 1;
		dpm->lclk.div = 1 + pgm;

		dpm->rclk.mul = 1;
/*		dpm->rclk.div = (virge->flags & VIRGE_CF_PERMEDIA2) 
**			? 1 : dpm->lclk.div;
*/		dpm->rclk.div = dpm->lclk.div;

		if (cmd == KGI_TC_LOWER) {

			if (dpm->dclk < virge->chipset.dclk.min) {

				KRN_DEBUG(1, "DCLK = %i Hz is too low", 
					dpm->dclk);
				return -E(CHIPSET, UNKNOWN);
			}

			if (lclk > 50000000) {

				dpm->dclk = 50000000 * dpm->lclk.div / 
					dpm->lclk.mul;
			}

		} else {

			if (lclk > 50000000) {

				KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
				return -E(CHIPSET, UNKNOWN);
			}
		}
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK");

#warning DO PROPER CHECKING!!!

		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

		if (width != img[0].virt.x) {

			KRN_DEBUG(2, "Invalid width!");
			return -E(CHIPSET, INVAL);
		}
		
		if ((img[0].size.x >= virge->chipset.maxdots.x) ||
			(img[0].size.y >= virge->chipset.maxdots.y) || 
			(img[0].virt.x >= virge->chipset.maxdots.x) ||
			((img[0].virt.y * img[0].virt.x * bpp) >
			(8 * virge->chipset.memory))) {

			KRN_DEBUG(1, "Resolution too high: %ix%i (%ix%i)",
				img[0].size.x, img[0].size.y, 
				img[0].virt.x, img[0].virt.y);
			return -E(CHIPSET, INVAL);
		}

		if (((dpm->lclk.mul != 1) && (dpm->lclk.div != 1 + pgm)) || 
			((dpm->rclk.mul != dpm->lclk.mul) && 
			 (dpm->rclk.div != dpm->lclk.div))) {

			KRN_DEBUG(1, "invalid LCLK (%i:%i) or CLK (%i:%i)", 
				dpm->lclk.mul, dpm->lclk.div, 
				dpm->rclk.mul, dpm->rclk.div);
			return -E(CHIPSET, INVAL);
		}

		if (lclk > 50000000) {

			KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
			return -E(CHIPSET, CLK_LIMIT);
		}

		if (img[0].flags & KGI_IF_STEREO) {

			KRN_DEBUG(1, "stereo modes not supported on virge");
			return -E(CHIPSET, NOSUP);
		}
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -E(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
#warning cleanup the C++ style comments!
//	virge_mode->VIRGE.VideoControl = VIRGE_VC058_EnableGPVideo | VIRGE_VC058_VSyncActive | VIRGE_VC058_HSyncActive | ((crt_mode->x.polarity > 0) ? VIRGE_VC058_HSyncHigh : VIRGE_VC058_HSyncLow) | ((crt_mode->y.polarity > 0) ? VIRGE_VC058_VSyncHigh : VIRGE_VC058_VSyncLow) | (((virge->flags & virge_CF_PERMEDIA2) && (kgim_attr_bits(img[0].bpfa) == 8)) ? 0 : VIRGE_VC058_Data64Enable);

//	virge_mode->virge.ScreenBase = 0;
	
//	if ((img[0].flags & KGI_IF_STEREO) && (virge->flags & VIRGE_CF_PERMEDIA2)) 
//	{
//		kgi_u_t 
//		Bpf = ((img[0].virt.x * img[0].virt.y * bpd) / 8 + 7) & ~7;

//		virge_mode->VIRGE.VideoControl |= VIRGE_VC058_StereoEnable;
//		virge_mode->VIRGE.ScreenBaseRight = ((1 == img[0].frames) && (2*bpf == bpd)) ? virge_mode->virge.ScreenBase : virge_mode->virge.ScreenBase + Bpf;
//	}

	mul = dpm->lclk.mul;
	div = dpm->lclk.div;
	bpd = kgim_attr_bits(dpm->bpda);

	/* Based on 64bit units */
//	virge_mode->VIRGE.ScreenStride = img[0].virt.x * mul / div;
	
//	if ((virge->flags & VIRGE_CF_PERMEDIA) || ((virge->flags & VIRGE_CF_PERMEDIA2) && (bpd == 8))) 
//	{
//		virge_mode->virge.ScreenStride /= 2;
//	}

	/* Based on LCLKs (32bit or 64bit units) */
	virge_mode->virge.HTotal = (crt_mode->x.total * mul / div) - 2;
	virge_mode->virge.HgEnd = ((crt_mode->x.total - crt_mode->x.width) * mul / div) - 1;
	virge_mode->virge.HbEnd = ((crt_mode->x.total - crt_mode->x.width) * mul / div) - 1;
	virge_mode->virge.HsStart = (crt_mode->x.syncstart - crt_mode->x.width) * mul / div;
	virge_mode->virge.HsEnd = (crt_mode->x.syncend - crt_mode->x.width) * mul / div;

	/* For some reason the video unit seems to delay the hsync pulse */
	shift = (((dpm->dclk / 10000) * 100) / 10000) * mul / div;
	if (virge_mode->virge.HsStart < shift) {

		shift = virge_mode->virge.HsStart;
	}
	
	virge_mode->virge.HsStart -= shift;
	virge_mode->virge.HsEnd -= shift;

	/* Based on lines */
	virge_mode->virge.VTotal	= crt_mode->y.total - 2;
	virge_mode->virge.VsStart	= crt_mode->y.syncstart - crt_mode->y.width;
	virge_mode->virge.VsEnd	= crt_mode->y.syncend - crt_mode->y.width - 1;
	virge_mode->virge.VbEnd	= crt_mode->y.total - crt_mode->y.width - 1;

	/* Initialize exported resources */
	r = &virge_mode->virge.aperture1;
	r->meta = virge;
	r->meta_io = virge_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "virge framebuffer";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = virge->chipset.memory;
	r->win.bus  = virge_io->aperture.base_bus;
	r->win.phys = virge_io->aperture.base_phys;
	r->win.virt = virge_io->aperture.base_virt;
	
	return KGI_EOK;
}

kgi_resource_t *virge_chipset_mode_resource(virge_chipset_t *virge, 
	virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index)
{
	KRN_DEBUG(2, "virge_chipset_mode_resource()");
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_mode_resource(&virge->vga, &virge_mode->vga, img, images, index);
	}

	switch (index) 
	{
	case 0:	
		KRN_DEBUG(2, "Returning aperture1 as a resource");
		return	(kgi_resource_t *) &virge_mode->virge.aperture1;
	case 1:	
		KRN_DEBUG(2, "Returning aperture2 as a resource");
		return	(kgi_resource_t *) &virge_mode->virge.aperture2;
	}
	
	KRN_DEBUG(2, "Returning NULL, index = %d", index);
	
	return NULL;
}

void virge_chipset_mode_prepare(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "virge_chipset_mode_prepare()");

	virge_chipset_sync(virge_io);
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		virge_io->flags |= VIRGE_IF_VGA_DAC;

//		VIRGE_MC_OUT32(virge_io, -1, VIRGE_MC_BypassWriteMask);
//		VIRGE_CS_OUT32(virge_io, VIRGE_CS050_ControllerVGA, 0x050);
//		VIRGE_CS_OUT32(virge_io, (VIRGE_CS_IN32(virge_io, 0x070) |
//			VIRGE_CS070_VGAEnable) & ~VIRGE_CS070_VGAFixed, 0x070);
//		kgi_chipset_vga_seq_out8(&(virge_io->vga), SR05_VGAEnableDisplay |
//					 SR05_EnableHostMemoryAccess |
//					 SR05_EnableHostDACAccess, 0x05);
		vga_text_chipset_mode_prepare(&virge->vga, &virge_io->vga,
					 &virge_mode->vga, img, images);

		KRN_DEBUG(2, "Prepared for VGA-mode");
		return;
	}

	virge_io->flags &= ~VIRGE_IF_VGA_DAC;

//	VIRGE_MC_OUT32(virge_io, -1, VIRGE_MC_BypassWriteMask);
//	VIRGE_CS_OUT32(virge_io, VIRGE_CS050_ControllerMem, 0x050);
//	VIRGE_CS_OUT32(virge_io, VIRGE_CS_IN32(virge_io, 0x070) &
//		~VIRGE_CS070_VGAEnable, 0x070);
//	VGA_SEQ_OUT8(&(virge_io->vga), 0, 0x05);

	KRN_DEBUG(2, "prepared for virge mode");
}

void virge_chipset_mode_enter(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "virge_chipset_mode_enter()");

	vga_text_chipset_mode_enter(&virge->vga, &virge_io->vga, &virge_mode->vga, img, images);
	KRN_DEBUG(2, "After vga_text_chipset_mode_enter()");
	
#if 0	
	int DCLKdiv = 8;
	int xtotal, xwidth, xblankstart, xblankend, xsyncstart, xsyncend,
	    ytotal, ywidth, yblankstart, yblankend, ysyncstart, ysyncend,
	    offset, bpp;

//	memcpy(&newmode, &reg_template, sizeof(struct riva_regs));

//	priv->newmode = &newmode;

//	KRN_DEBUG(2, "Setting mode... (%dx%d %d)", mode->request.virt.x, mode->request.virt.y, mode->request.graphtype);

	xtotal = (mode->x.total / DCLKdiv) - 5;
	xwidth = (mode->x.width / DCLKdiv) - 1;
	xblankstart = mode->x.blankstart / DCLKdiv;
	xblankend = mode->x.blankend / DCLKdiv;
	xsyncstart = mode->x.syncstart / DCLKdiv;
	xsyncend = mode->x.syncend / DCLKdiv;

	ytotal = mode->y.total - 2;
	ywidth = mode->y.width - 1;
	yblankstart = mode->y.blankstart - 1;
	yblankend = mode->y.blankend - 1;
	ysyncstart = mode->y.syncstart - 1;
	ysyncend = mode->y.syncend - 1;

	switch(mode->request.graphtype) {

	case KGIGT_8BIT: bpp = 8; break;
	case KGIGT_16BIT: bpp = 16; break;
	case KGIGT_32BIT: bpp = 32; break;
	default:
		INTERNAL_ERROR;
		return;
	}

	offset = (mode->request.virt.x/8) * (bpp/8);

	virge->crt.htotal = xtotal & 0xFF;
	virge->crt.hdisplayend = xwidth & 0xFF;
	virge->crt.hblankstart = xblankstart & 0xFF;
	virge->crt.hblankend |= xblankend & 0x1F;
	virge->crt.hretracestart = xsyncstart & 0xFF;
	virge->crt.hretraceend = (xsyncend & 0x1F) | SetBitField(xblankend, 5:5, NV_PCRTC_HORIZ_RETRACE_END_HORIZ_BLANK_END_5);

	/* Vertical */
	virge->crt.vdisplayend = ywidth & 0xFF;
	virge->crt.vblankstart = yblankstart & 0xFF;
	virge->crt.vretracestart = ysyncstart & 0xFF;
	virge->crt.vretraceend = ysyncend & 0xFF;
	virge->crt.vblankend = yblankend & 0xFF;
	virge->crt.vtotal = ytotal & 0xFF;

	/* Other */

	virge->crt.overflow =
		SetBitField( ysyncstart,  9:9, NV_PCRTC_OVERFLOW_VERT_RETRACE_START_9) |
		SetBitField( ywidth,	  9:9, NV_PCRTC_OVERFLOW_VERT_DISPLAY_END_9) |
		SetBitField( ytotal,      9:9, NV_PCRTC_OVERFLOW_VERT_TOTAL_9) |
		SetBitField( 1,		  0:0, NV_PCRTC_OVERFLOW_LINE_COMPARE_8) |
		SetBitField( yblankstart, 8:8, NV_PCRTC_OVERFLOW_VERT_BLANK_START_8) |
		SetBitField( ysyncstart,  8:8, NV_PCRTC_OVERFLOW_VERT_RETRACE_START_8) |
		SetBitField( ywidth,      8:8, NV_PCRTC_OVERFLOW_VERT_DISPLAY_END_8) |
		SetBitField( ytotal,      8:8, NV_PCRTC_OVERFLOW_VERT_TOTAL_8);

	virge->crt.offset = offset & 0xFF;

	virge->crt.maxscanline =
		SetBitField( (mode->magnify.y == 2), 0:0, NV_PCRTC_MAX_SCAN_LINE_DOUBLE_SCAN) |
		SetBitField(1,             	     0:0, NV_PCRTC_MAX_SCAN_LINE_LINE_COMPARE_9) |
		SetBitField(yblankstart,	     9:9, NV_PCRTC_MAX_SCAN_LINE_VERT_BLANK_START_9) |
		SetBitField(0,			     4:0, NV_PCRTC_MAX_SCAN_LINE_MAX_SCAN_LINE);

//        newmode.misc_output |=
//		((mode->x.polarity > 0) ? 0 : MISC_NEG_HSYNC) |
//		((mode->y.polarity > 0) ? 0 : MISC_NEG_VSYNC);

	/* Ok.  We'll do the actual hardware programming now. */

//        kgim_clock_set_mode(dpy, mode);

	riva.CalcStateExt(&riva, &newmode.ext, bpp, mode->request.virt.x,
			   mode->x.width, xwidth, xsyncstart, xsyncend,
			   xtotal, mode->request.virt.y, ywidth, ysyncstart,
			   ysyncend, ytotal,
			   priv->clock / 1000); /* the clock is calculated in kgim_clock_set_mode */

//	ENTER_CRITICAL;
	
//	load_state(dpy, &newmode);
//	kgim_monitor_set_mode(dpy, mode);
//	kgim_ramdac_set_mode(dpy, mode);
//	kgim_accel_set_mode(dpy, mode);
	
//	LEAVE_CRITICAL;

//	NOTICE2("setting up display...");

//	chipset_setup_dpy(dpy, mode);

//	NOTICE1("Mode set complete");
	
#if DEBUG_LEVEL > 1
	KRN_DEBUG(1, "Newly programmed state:");
	dump_state(dpy, &newmode);
#endif
#endif	
	KRN_DEBUG(2, "Before...");
	virge->mode = virge_mode;
	KRN_DEBUG(2, "...After.");
}

void virge_chipset_mode_leave(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "virge_chipset_mode_leave()");
	
	virge_chipset_sync(virge_io);

	virge->mode = NULL;
}
