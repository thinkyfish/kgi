/* **********************************************************************
 *
 *  A P I  -  Matrox accelerated drawing functions
 *
 * **********************************************************************/

#ifndef _MGA_ACCEL_H
#define _MGA_ACCEL_H

/*
**    Needed header files
*/
#include <kgi/kgi.h>
#include "mga_accel_ring.h"
#include <Matrox/Gx00.h>

/* ******************************************
 *   Accelerator state data structures
 * ******************************************/

typedef struct _mga_accel_state_s mga_accel_state_t;
typedef int bool;

/* ******************************************
 *   Accelerator operations
 * ******************************************/

/*
 * General operations
 */
/* Initialization */
static void mga_accel_init(mga_accel_state_t *state,
			   mga_dma_ring_t *ring,
			   kgi_image_mode_t *mode);
/* Flush the pending commands */
static inline void mga_accel_flush(mga_accel_state_t *state);

/*
 * Drawing state modification operations
 */
/* Foreground/background color */
static inline void mga_accel_set_foreground(mga_accel_state_t *state,
					    mga_vertex_color_t col);
static inline void mga_accel_set_background(mga_accel_state_t *state,
					    mga_vertex_color_t col);
/* Drawing command */
static inline void mga_accel_set_dwg_cmd(mga_accel_state_t *state,
					 kgi_u32_t cmd);
/* Clipping */
static void mga_accel_set_clip(mga_accel_state_t *state,
			       int left, int top, int right, int bottom);
/* Textures */
static inline void mga_accel_set_texstage(mga_accel_state_t *state,
					  kgi_u32_t tstage0,
					  kgi_u32_t tstage1);
static inline void mga_accel_set_texture(mga_accel_state_t *state, kgi_u_t no);

/*
 * Drawing operations
 */
static inline void mga_accel_draw_box(mga_accel_state_t *state,
				      int x, int y, int w, int h);
static inline void mga_accel_draw_line(mga_accel_state_t *state,
				       int x, int y, int xe, int ye);
static inline void mga_accel_draw_triangle(mga_accel_state_t *state,
					   mga_vertex_t *v1,
					   mga_vertex_t *v2,
					   mga_vertex_t *v3);
static inline void mga_accel_draw_tex_triangle(mga_accel_state_t *state,
					       mga_vertex_t *v1,
					       mga_vertex_t *v2,
					       mga_vertex_t *v3);


/* **********************************************************************
 *
 *  I M P L E M E N T A T I O N  -  Matrox accelerated drawing functions
 *
 * **********************************************************************/

/* ******************************************
 *   Accelerator state data structures
 * ******************************************/

static bool false = 0;
static bool true = 1;

struct _mga_accel_state_s
{
  /* user-level state information */
  kgi_u_t bpp;
  kgi_u_t screen_width;
  struct
  {
    kgi_u_t top;
    kgi_u_t bottom;
    kgi_u_t left;
    kgi_u_t right;
  } max_clip;
  kgi_u32_t free_space_offset;

  /* Drawing-related state information */
  kgi_u32_t foreground; /* FCOL */
  kgi_u32_t background; /* BCOL */
  bool do_clipping;
  struct
  {
    kgi_u_t top;
    kgi_u_t bottom;
    kgi_u_t left;
    kgi_u_t right;
  } clip;
  kgi_u32_t zbuffer; /* ZORG */

  /* texture related */
  kgi_s_t tex_no;

  /*
   * Hardware state information
   */
  struct {
    /* mode related */
    kgi_u32_t sMACCESS;
    kgi_u32_t sZORG;
    kgi_u32_t sPITCH;
    /* drawing related */
    kgi_u32_t sFCOL;
    kgi_u32_t sBCOL;
    kgi_u32_t sCXBNDRY;
    kgi_u32_t sYTOP;
    kgi_u32_t sYBOT;
    kgi_u32_t sDSTORG; /* G200+ */
    kgi_u32_t sYDSTORG; /* Mystique */
    kgi_u32_t sPLNWT;
    kgi_u32_t sDWGCTL;
    /* texture related */
    kgi_u32_t sTDUALSTAGE0;
    kgi_u32_t sTDUALSTAGE1;
    
  } hw_state;

  mga_dma_ring_t *ring;

};

#define _MGA_STATE_REG(state, reg) ((state)->hw_state.s##reg)

#define _MGA_STATE_OUT_REG(state, value, reg)     \
 mga_dma_out_reg((state)->ring, (value), (reg));  \
 (state)->hw_state.s##reg = (value);

#define _MGA_STATE_RESTORE_REG(state, reg) \
 mga_dma_out_reg((state)->ring, (state)->hw_state.s##reg, (reg));

/*
 * Miscellaneous functions
 */
static kgi_u_t _kgim_attr_bits(const kgi_u8_t *bpa)
{
  kgi_u_t bits = 0;

  if (bpa)
    while (*bpa)
      bits += *(bpa++);
  return bits;
}
static kgi_u_t _kgim_bpp(kgi_image_mode_t *img)
{
  kgi_u_t bpf, bpc;
  bpf = _kgim_attr_bits(img->bpfa);
  bpc = _kgim_attr_bits(img->bpca);
  return ((img->flags & KGI_IF_STEREO) ?
	  (bpc + bpf*img->frames*2) :
	  (bpc + bpf*img->frames));
}

/*
 * Init & management functions
 */
static inline void _mga_accel_send_state(mga_accel_state_t *state)
{
#if 0
  mga_dma_check(state->ring, 13, MGA_DMA_GENERAL_PURPOSE);

  _MGA_STATE_RESTORE_REG(state, MACCESS);
  _MGA_STATE_RESTORE_REG(state, ZORG);
  _MGA_STATE_RESTORE_REG(state, PITCH);
  _MGA_STATE_RESTORE_REG(state, FCOL);
  _MGA_STATE_RESTORE_REG(state, BCOL);
  _MGA_STATE_RESTORE_REG(state, CXBNDRY);
  _MGA_STATE_RESTORE_REG(state, YTOP);
  _MGA_STATE_RESTORE_REG(state, YBOT);
#if 1
  _MGA_STATE_RESTORE_REG(state, DSTORG); /* G200+ */
#else
  _MGA_STATE_RESTORE_REG(state, YDSTORG); /* Mystique */
#endif
  _MGA_STATE_RESTORE_REG(state, PLNWT);
  _MGA_STATE_RESTORE_REG(state, DWGCTL);
  _MGA_STATE_RESTORE_REG(state, TDUALSTAGE0);
  _MGA_STATE_RESTORE_REG(state, TDUALSTAGE1);
#endif
}

static void mga_accel_flush(mga_accel_state_t *state)
{
  mga_dma_flush(state->ring, MGA_DMA_GENERAL_PURPOSE);
  _mga_accel_send_state(state);
}

static void mga_accel_init(mga_accel_state_t *state,
			   mga_dma_ring_t *ring,
			   kgi_image_mode_t *mode)
{
  kgi_u32_t maccess;

  state->ring = ring;

  state->bpp = _kgim_bpp(mode);
  state->screen_width = mode->virt.x;

  /* user-level state */
  state->foreground = 0xFFFFFFFF; /* usually white */
  state->background = 0x00000000; /* usually black */
  state->do_clipping = false;
  state->clip.left = state->max_clip.left = 0;
  state->clip.right = state->max_clip.right = mode->virt.x;
  state->clip.top = state->max_clip.top = 0;
  state->clip.bottom = state->max_clip.bottom = mode->virt.y;
  state->zbuffer = mode->virt.x * mode->virt.y * (state->bpp / 8);
  state->tex_no = -1;

  /* hardware state */
  _MGA_STATE_OUT_REG(state,state->foreground,FCOL);
  _MGA_STATE_OUT_REG(state,state->background,BCOL);
  _MGA_STATE_OUT_REG(state,(state->zbuffer & ~0x3),ZORG);
  _MGA_STATE_OUT_REG(state,
		    ((state->clip.left << CXBNDRY_CXLEFT_SHIFT)
		     & CXBNDRY_CXLEFT_MASK)
		    | ((state->clip.right << CXBNDRY_CXRIGHT_SHIFT)
		       & CXBNDRY_CXRIGHT_MASK),
		    CXBNDRY);
  _MGA_STATE_OUT_REG(state,state->clip.top * mode->virt.x * (state->bpp / 8),
		     YTOP);
  _MGA_STATE_OUT_REG(state,state->clip.bottom * mode->virt.x * (state->bpp / 8),
		     YBOT);
#if MYSTIQUE
  _MGA_STATE_OUT_REG(state,0,YDSTORG);
#else
  _MGA_STATE_OUT_REG(state,0,DSTORG);
#endif
#if 1  /* Test to see if in-driver init works */
  _MGA_STATE_OUT_REG(state,mode->virt.x,PITCH);
#endif
  _MGA_STATE_OUT_REG(state,0xFFFFFFFF,PLNWT);

#if 1 /* Test to see if in-driver init works */
  /* mode-related settings */
  maccess = MACCESS_ZWIDTH_ZW16; // | MACCESS_NODITHER;
  switch (state->bpp)
    {
    case 8:
      maccess |= MACCESS_PWIDTH_PW8;
      break;
    case 15:
      maccess |= MACCESS_DIT555;
      maccess |= MACCESS_PWIDTH_PW16;
      break;
    case 16:
      maccess |= MACCESS_PWIDTH_PW16;
      break;
    case 24:
      maccess |= MACCESS_PWIDTH_PW24;
      break;
    case 32:
      maccess |= MACCESS_PWIDTH_PW32;
      break;
    }
  _MGA_STATE_OUT_REG(state,maccess,MACCESS);
#endif
  _MGA_STATE_OUT_REG(state,0,TDUALSTAGE0);
  _MGA_STATE_OUT_REG(state,0,TDUALSTAGE1);

  mga_dma_check(state->ring, 2, MGA_DMA_GENERAL_PURPOSE);
  mga_dma_out_reg(state->ring, WACCEPTSEQ_SEQOFF | WACCEPTSEQ_WSAMETAG,
		  WACCEPTSEQ);
  mga_dma_out_reg(state->ring, 1, ALPHACTRL);

  /* other user level state */
  state->free_space_offset = state->zbuffer
    + mode->virt.x * mode->virt.y * 2; /* 2 bytes per Z-buffer entry */
  state->free_space_offset += (256 / 8); /* Rounds to 256-bit boundary */
  state->free_space_offset &= ~0x1F;

  mga_accel_flush(state);
}

/* NB: we use the vertex color type as a workaround for the
 * moment
 */
static inline kgi_u32_t _vertexcolor2accel(mga_accel_state_t *state,
					  mga_vertex_color_t color)
{
  kgi_u32_t ret;
  kgi_u32_t r,g,b,a;

  /* avoids type expansion problems (useful?) */
  b = color.blue;
  g = color.green;
  r = color.red;
  a = color.alpha;
  switch (state->bpp)
    {
    case 8:
      ret = b;
      break;
    case 15:
      b = b / (256 / 32);
      r = r / (256 / 32);
      g = g / (256 / 32);
      a = a >> 7; /* high order bit only */
      ret = (b & 0x1F) | ((g & 0x1F) << 5) | ((r & 0x3F) << 10)
	| ((a & 1) << 15);
      break;
    case 16:
      b = b / (256 / 32);
      r = r / (256 / 32);
      g = g / (256 / 64);
      ret = (b & 0x1F) | ((g & 0x3F) << 5) | ((r & 0x3F) << 11);
      break;
    case 24:
      ret = b | (g << 8) | (r << 16);
      break;
    case 32:
      ret = b | (g << 8) | (r << 16) | (a << 24);
      break;
    }
  return ret;
}

static void mga_accel_set_foreground(mga_accel_state_t *state,
				     mga_vertex_color_t col)
{
  kgi_u32_t fcol = _vertexcolor2accel(state,col);
  if (fcol != state->foreground)
    {
      state->foreground = fcol;
      mga_dma_check(state->ring, 1, MGA_DMA_GENERAL_PURPOSE);
      _MGA_STATE_OUT_REG(state, fcol, FCOL);
    }
}

static void mga_accel_set_background(mga_accel_state_t *state,
				     mga_vertex_color_t col)
{
  kgi_u32_t bcol = _vertexcolor2accel(state,col);
  if (bcol != state->background)
    {
      state->background = bcol;
      mga_dma_check(state->ring, 1, MGA_DMA_GENERAL_PURPOSE);
      _MGA_STATE_OUT_REG(state, bcol, BCOL);
    }
}

static void mga_accel_set_dwg_cmd(mga_accel_state_t *state,
				  kgi_u32_t cmd)
{
  if (cmd != _MGA_STATE_REG(state,DWGCTL))
    {
      mga_dma_check(state->ring, 1, MGA_DMA_GENERAL_PURPOSE);
      _MGA_STATE_OUT_REG(state, cmd, DWGCTL);
    }
}

static void mga_accel_set_clip(mga_accel_state_t *state,
			       int left, int top, int right, int bottom)
{
  state->clip.left = (left < state->max_clip.left)
    ? state->max_clip.left : left;
  state->clip.top = (top < state->max_clip.top)
    ? state->max_clip.top : top;
  state->clip.right = (right > state->max_clip.right)
    ? state->max_clip.right : right;
  state->clip.bottom = (bottom > state->max_clip.bottom)
    ? state->max_clip.bottom : bottom;

  mga_dma_check(state->ring, 3, MGA_DMA_GENERAL_PURPOSE);
  _MGA_STATE_OUT_REG(state,
		    ((state->clip.left << CXBNDRY_CXLEFT_SHIFT)
		     & CXBNDRY_CXLEFT_MASK)
		    | ((state->clip.right << CXBNDRY_CXRIGHT_SHIFT)
		       & CXBNDRY_CXRIGHT_MASK),
		    CXBNDRY);
  _MGA_STATE_OUT_REG(state, state->clip.top * state->screen_width * (state->bpp / 8),
		     YTOP);
  _MGA_STATE_OUT_REG(state, state->clip.bottom * state->screen_width * (state->bpp / 8),
		     YBOT);
}

static void mga_accel_set_texstage(mga_accel_state_t *state,
				   kgi_u32_t tstage0, kgi_u32_t tstage1)
{
#if CHIPSET_G200

  /* Nothing */

#elif CHIPSET_G400

  if ((tstage0 != _MGA_STATE_REG(state,TDUALSTAGE0))
      || (tstage1 != _MGA_STATE_REG(state,TDUALSTAGE1)))
    {
      mga_dma_check(state->ring, 2, MGA_DMA_GENERAL_PURPOSE);
      _MGA_STATE_OUT_REG(state, tstage0, TDUALSTAGE0);
      _MGA_STATE_OUT_REG(state, tstage1, TDUALSTAGE1);
    }

#elif CHIPSET_MYSTIQUE

  /* Nothing */

#else
#error Unknown chipset!
#endif /* CHIPSET_{G{2,4}00,MYSTIQUE} */
}

static void mga_accel_set_texture(mga_accel_state_t *state, kgi_u_t no)
{
  if (no != state->tex_no)
    {
#if CHIPSET_G200

      kgi_u32_t tex_offset = state->free_space_offset + 512 * no;
      //kgi_u32_t tex_offset = no * 1024 * 2 - 32 * 2;
      kgi_u32_t texwidth = (4 + 24 + 4) /* tw, log2(16) = 4 */
	| 0x40 /* see: warp DDK */
	| (((10 - 4 - (24 - 16))&63) << 9) /* rfw = 10 - log2(16) - (24 - 16) */
	| (15 << 18) /* twmask = 16 - 1 */
	;
      kgi_u32_t texheight = texwidth;

      mga_dma_check(state->ring, (12+7), MGA_DMA_GENERAL_PURPOSE);

      mga_dma_out_reg(state->ring,
		      0x3 /* tformat = TW16 */
		      /* tpitchlin = 0 */
		      | (0x1 << 16) /* tpitch = 16 */
		      //| (1 << 27) | (1 << 28) /* clampu,v */
		      | (1 << 29) /* tmodulate */
		      //| (1 << 30) | (1 << 24) /* strans and decalckey */
		      | (1 << 25) /* takey */
		      ,
		      TEXCTL);
      //mga_dma_out_reg(state->ring, 0x24, TEXCTL2);
      mga_dma_out_reg(state->ring, 1 /* | (1 << 5) */, TEXCTL2);
      
      mga_dma_out_reg(state->ring, texwidth, TEXWIDTH);
      mga_dma_out_reg(state->ring, texheight, TEXHEIGHT);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG);
#if 0
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG1);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG2);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG3);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG4);
#endif
      mga_dma_out_reg(state->ring, (0x0 << 16) | 0xFFFF, TEXTRANS);
      mga_dma_out_reg(state->ring, 0xFFFF, TEXTRANSHIGH);
      mga_dma_out_reg(state->ring, 0x0 | (0x2 << 4) | (0x0F << 21)
		      , TEXFILTER);

#if 1
      mga_dma_out_reg(state->ring, (1 << 22) | (1 << 23),
		      WFLAG); /* repeat */
      mga_dma_out_reg(state->ring, 0x100, WARPREG(25));
      mga_dma_out_reg(state->ring, texwidth | 0x40, WARPREG(24));
      mga_dma_out_reg(state->ring, texheight | 0x40, WARPREG(34));
      mga_dma_out_reg(state->ring, 0xFFFF, WARPREG(42));
      mga_dma_out_reg(state->ring, 0xFFFF, WARPREG(60));
#endif

#elif CHIPSET_G400

      kgi_u32_t tex_offset = state->free_space_offset + 512 * no;
      kgi_u32_t texwidth = (4 + 24 - 17 + 4) /* tw, log2(16) = 4 */
	| 0x40 /* see: warp DDK */
	| (((10 - 4 - (24 - 16))&63) << 9) /* rfw = 8 - log2(16) - (16 - 16) */
	| (15 << 18) /* twmask = 16 - 1 */
	;
      kgi_u32_t texheight = texwidth;

      mga_dma_check(state->ring, (12+9), MGA_DMA_GENERAL_PURPOSE);

      mga_dma_out_reg(state->ring,
		      0x3 /* tformat = TW16 */
		      /* tpitchlin = 0 */
		      | (0x1 << 16) /* tpitch = 16 */
		      //| (1 << 27) | (1 << 28) /* clampu,v */
		      | (1 << 29) /* tmodulate */
		      //| (1 << 30) | (1 << 24) /* strans and decalckey */
		      | (1 << 25) /* takey */
		      ,
		      TEXCTL);
      //mga_dma_out_reg(state->ring, 0x24, TEXCTL2);
      mga_dma_out_reg(state->ring, 1 /* | (1 << 5) */, TEXCTL2);
      
      mga_dma_out_reg(state->ring, texwidth, TEXWIDTH);
      mga_dma_out_reg(state->ring, texheight, TEXHEIGHT);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG);
#if 0
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG1);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG2);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG3);
      mga_dma_out_reg(state->ring, (tex_offset & ~0x1F), TEXORG4);
#endif
      mga_dma_out_reg(state->ring, (0x0 << 16) | 0xFFFF, TEXTRANS);
      mga_dma_out_reg(state->ring, 0xFFFF, TEXTRANSHIGH);
      mga_dma_out_reg(state->ring, 0x0 | (0x2 << 4) | (0x0F << 21)
		      , TEXFILTER);

#if 1
      mga_dma_out_reg(state->ring, (1 << 22) | (1 << 23),
		      WFLAG); /* repeat */
      mga_dma_out_reg(state->ring, (1 << 22) | (1 << 23), WFLAG1);
      mga_dma_out_reg(state->ring, 0, WARPREG(49));
      mga_dma_out_reg(state->ring, 0, WARPREG(57));
      mga_dma_out_reg(state->ring, texwidth | 0x40, WARPREG(54));
      mga_dma_out_reg(state->ring, texheight | 0x40, WARPREG(62));
      mga_dma_out_reg(state->ring, 0, WARPREG(53));
      mga_dma_out_reg(state->ring, 0, WARPREG(61));
      mga_dma_out_reg(state->ring, texwidth | 0x40, WARPREG(52));
#endif

#elif CHIPSET_MYSTIQUE

#warning Texture support not yet implemented on Mystique!

#else
#error Unknown chipset!
#endif /* CHIPSET_G{2,4}00 */
      state->tex_no = no;
    }
}

static void mga_accel_draw_box(mga_accel_state_t *state,
			       int x, int y, int w, int h)
{
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if ((w <= 0) || (h <= 0)) return;

  mga_accel_set_dwg_cmd(state,
			DWGCTL_OPCOD_TRAP | DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
			| DWGCTL_SOLID | DWGCTL_ARZERO | DWGCTL_SGNZERO
			| DWGCTL_SHFTZERO
			| ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
			| ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
			| DWGCTL_BLTMOD_BFCOL
			/* patterning disabled */
			| DWGCTL_TRANSC
			| (state->do_clipping ? 0 : DWGCTL_CLIPDIS));

  mga_dma_check(state->ring, 2, MGA_DMA_GENERAL_PURPOSE);

  mga_dma_out_reg(state->ring, (x & 0xFFFF) | (((x + w) & 0xFFFF) << 16),
		  FXBNDRY);
  mga_dma_out_reg(state->ring, (h & 0xFFFF) | ((y & 0xFFFF) << 16),
		  YDSTLEN | ACCEL_GO);
}

static void mga_accel_draw_line(mga_accel_state_t *state,
				int x, int y, int xe, int ye)
{

  if (x > xe) { int tmp; tmp = x; x = xe; xe = tmp; }
  if (y > ye) { int tmp; tmp = y; y = ye; ye = tmp; }

  mga_accel_set_dwg_cmd(state,
			DWGCTL_OPCOD_AUTOLINE_CLOSE
			| DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
			| DWGCTL_SOLID
			| DWGCTL_SHFTZERO
			| ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
			| ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
			| DWGCTL_BLTMOD_BFCOL
			/* patterning disabled */
			/* translucency disabled */
			| (state->do_clipping ? 0 : DWGCTL_CLIPDIS));
  
  mga_dma_check(state->ring, 2, MGA_DMA_GENERAL_PURPOSE);

  mga_dma_out_reg(state->ring, (x & 0xFFFF) | ((y & 0xFFFF) << 16),
		  XYSTRT);
  mga_dma_out_reg(state->ring, (xe & 0xFFFF) | ((ye & 0xFFFF) << 16),
		  XYEND | ACCEL_GO);
}

static void mga_accel_draw_triangle(mga_accel_state_t *state,
				    mga_vertex_t *v1,
				    mga_vertex_t *v2,
				    mga_vertex_t *v3)
{
#if CHIPSET_MYSTIQUE

#warning Warp-based triangle drawing not available on Mystique!

#else

  /* cf p.4-42, second note */
  mga_accel_set_texstage(state, 0x3 << 21, 0x3 << 21);

  mga_accel_set_dwg_cmd(state,
			DWGCTL_OPCOD_TRAP
			//| DWGCTL_ATYPE_I | DWGCTL_ZMODE_NOZCMP
			| DWGCTL_ATYPE_ZI | DWGCTL_ZMODE_ZLTE
			| DWGCTL_SHFTZERO
			| ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
			| ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
			/* no bltmod specified */
			/* patterning disabled */
			/* translucency disabled */
			| DWGCTL_CLIPDIS);
  //| (state->do_clipping ? 0 : DWGCTL_CLIPDIS));
  /* We send only ONE triangle then flush the buffer */
  mga_dma_check(state->ring, 1, MGA_DMA_VERTEX_TRIANGLE_LIST);
  mga_dma_out_triangle(state->ring, v1, v2, v3);

#endif /* CHIPSET_MYSTIQUE */
}

static void mga_accel_draw_tex_triangle(mga_accel_state_t *state,
					mga_vertex_t *v1,
					mga_vertex_t *v2,
					mga_vertex_t *v3)
{
#if CHIPSET_MYSTIQUE

#warning Warp-based triangle drawing not available on Mystique!

#else

  //mga_dma_out_reg(state->ring, (0x2 << 2) | (1 << 10) | (1 << 12) | (1 << 11) | (1 << 17) | (0x2 << 21) | (0x1 << 30), TDUALSTAGE0);
  //mga_dma_out_reg(state->ring, (0x2 << 2) | (1 << 10) | (1 << 12) | (1 << 11) | (1 << 17) | (0x2 << 21) | (0x1 << 30), TDUALSTAGE1);
  mga_accel_set_texstage(state, 0, 0);

  mga_accel_set_dwg_cmd(state,
			DWGCTL_OPCOD_TEXTURE_TRAP
			//| DWGCTL_ATYPE_I | DWGCTL_ZMODE_NOZCMP
			| DWGCTL_ATYPE_ZI | DWGCTL_ZMODE_ZLTE
			| DWGCTL_SHFTZERO
			| ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
			| ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
			/* no bltmod specified */
			/* patterning disabled */
			/* translucency disabled */
			| (state->do_clipping ? 0 : DWGCTL_CLIPDIS));

  /* We send only ONE triangle then flush the buffer */
  mga_dma_check(state->ring, 1, MGA_DMA_VERTEX_TRIANGLE_LIST);
  mga_dma_out_triangle(state->ring, v1, v2, v3);

#endif /* CHIPSET_MYSTIQUE */
}

#endif /* _MGA_ACCEL_H */
