/* ----------------------------------------------------------------------------
**	GGI API and type definitions
** ----------------------------------------------------------------------------
**
**	Copyright (C)	1995-1997	Steffen Seeger
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen Seeger
**
**	$Log: ggi.h,v $
**	Revision 1.4  1998/04/18 16:33:24  seeger_s
**	- renamed GGI_PA_INDEX -> GGI_PA_COLOR_INDEX
**	
**	Revision 1.3  1998/04/04 22:42:55  degas
**	- headers, logging, system layer change
**	
** ----------------------------------------------------------------------------
*/

#ifndef	__ggi_ggi_h
#define	__ggi_ggi_h

#include <ggi/system.h>
#include <ggi/debug.h>

#define	__GGI_MAX_NR_FRAMES	8

__GGI_SYS_INT_TYPES(ggi)

typedef struct { ggi_s min, max; }	ggi_range;
typedef struct { ggi_s mul, div; }	ggi_ratio;
typedef struct { ggi_s16   x, y; }	ggi_coord;
typedef	struct { ggi_u16   x, y; }	ggi_size;
typedef	ggi_u				ggi_pixel;
typedef	ggi_u				ggi_zvalue;
typedef	ggi_u32				ggi_mask;
#define	GGI_MASK(x)	(((ggi_u32) 1) << (x))

/*	Rescale Flags
**
**	Rescale flags are used to specify how to treat pixmaps when rescaling 
**	them. We use ggi_u8 to store them, thus 16 methods are possible.
**	more will need a recompile of GGI applications.
*/
typedef enum ggi_rescale_flags {

	GGI_RF_CLIP_X	= 0x00,
	GGI_RF_CLAMP_X	= 0x01,
	GGI_RF_REPEAT_X	= 0x02,
	GGI_RF_SCALE_X	= 0x03,
	GGI_RF_MASK_X 	= 0x0f,

	GGI_RF_CLIP_Y	= 0x00,
	GGI_RF_CLAMP_Y	= 0x10,
	GGI_RF_REPEAT_Y	= 0x20,
	GGI_RF_SCALE_Y	= 0x30,
	GGI_RF_MASK_Y	= 0xf0

} ggi_rescale_flags;

/*	Pixel Textures
**
**	Pixel textures are the textures that are mapped on the screen on a per
**	pixel basis (probably better known as text fonts). They are monochrome,
**	represented by a MAX_PT_DOTS_X times MAX_PT_DOTS_Y bitmap which is
**	overlaid in the foreground color on the pixel background color.
** NOTE	Changing this requires a recompile of GGI applications!
*/
#define	MAX_PT_DOTS_X	32
#define	MAX_PT_DOTS_Y	32
typedef ggi_u32 ggi_pixel_texture[32];

typedef struct ggi_tlut {

	ggi_u		size;		/* number of items		*/
	ggi_size	dots;		/* size of textures		*/
	ggi_u		format;		/* how to interpret *data	*/

	ggi_u8		*rsf;		/* pixel texture rescale flags	*/
	ggi_void	*data;		/* texture data ('compressed')	*/
	ggi_unicode	*name;		/* 'meaning' of the textures	*/

} ggi_tlut;

/*	Colors
**
**	Colors can be represented differently, and there are many different
**	'color spaces'. However, we handle them all at 16 bit precision per
**	channel. For mono color space we use YUV model and code the
**	mono-chrome (the color) in u and v.
*/
typedef struct ggi_rgb_color { ggi_u16 r,g,b; }	ggi_rgb_color;
typedef struct ggi_yuv_color { ggi_u16 y,u,v; }	ggi_yuv_color;
typedef struct ggi_yrb_color { ggi_u16 y,r,b; }	ggi_yrb_color;
typedef	struct ggi_yrg_color { ggi_u16 y,r,g; }	ggi_yrg_color;
typedef	struct ggi_ygb_color { ggi_u16 y,g,b; }	ggi_ygb_color;

typedef union ggi_color {

	ggi_rgb_color	rgb;
	ggi_yuv_color	yuv;
	ggi_yrb_color	yrb;
	ggi_yrg_color	yrg;
	ggi_ygb_color	ygb;

} ggi_color;

typedef enum ggi_color_space {

	GGI_CS_LINEAR = 0x01000000,	/* linear intensity		*/

		GGI_CS_LIN_CHANNEL,	/* look up per channel		*/
		GGI_CS_LIN_MONO,	/* monochrome			*/
		GGI_CS_LIN_RGB,		/* red, green, blue		*/
		GGI_CS_LIN_YCRCB,	/* intensity, red, blue		*/

	GGI_CS_LOGARITHMIC = 0x02000000,/* logarithmic intensity	*/

		GGI_CS_LOG_CHANNEL,	/* look up per channel		*/
		GGI_CS_LOG_MONO,	/* monochrome			*/
		GGI_CS_LOG_RGB,		/* RGB model			*/
		GGI_CS_LOG_YCRCB,	/* YCrCb model			*/

	GGI_CS_INTENSITY = 0x7F000000	/* to extract intensity funct.	*/

} ggi_color_space;

typedef struct ggi_clut {

	ggi_u			size;	/* number of color entries	*/
	ggi_color_space		cspace;	/* color space of the entries	*/
	ggi_color		*c;	/* color data			*/

} ggi_clut;

/*	Attributes
**
**	Attributes are used to describe which properties of a viewport,
**	image or pixel can be controlled. We use three types of attributes
**		- scalar values (have current, min, max value)
**		- exclusive bit mask ('radiobutton' selection)
**		- nonexclusive bit mask ('checkbox' selection)
**
**	For each object, there may be up to __GGI_MAX_NR_PIXEL_ATTRIBUTES
**	properties currently. 
*/
#define	__GGI_MAX_NR_PIXEL_ATTRIBUTES	32

typedef ggi_u ggi_attribute;
typedef ggi_u ggi_attribute_mask;

/*	Pixel Attributes
**
**	These can be controlled per pixel of an image. Any of the attributes
**	may be missing, and there might be some not defined yet. To query
**	information about which attributes can be controlled, use the
**	ggiPixelAttributeMask() function that returns a pixel value bitmask
**	of the bits that are used to contol a given attribute. If a bitmask
**	of zero is returned, the attribute queried cannot be controlled.
*/
typedef enum ggi_pixel_attribute {

	/*
	**	These are actually observed/specified/implemented.
	*/
	GGI_PA_PRIVATE,		/* hardware or driver private		*/
	GGI_PA_APPLICATION,	/* application dependent		*/
	GGI_PA_STENCIL,		/* stencil buffer			*/
	GGI_PA_Z,		/* z value				*/
	GGI_PA_COLOR_INDEX,	/* color index				*/
	GGI_PA_COLOR1,		/* intensity of color channel 1		*/
	GGI_PA_COLOR2,		/* intensity of color channel 2		*/
	GGI_PA_COLOR3,		/* intensity of color channel 3		*/
	GGI_PA_ALPHA,		/* alpha value (all channels)		*/

	GGI_PA_FOREGROUND_INDEX,/* index of the color of the texture	*/
	GGI_PA_TEXTURE_INDEX,	/* index of the texture			*/
	GGI_PA_BLINK,		/* blink bit/frequency			*/

	/*
	**	these are hypothetic (not specified further).
	*/
	GGI_PA_ALPHA1,		/* alpha value channel 1		*/
	GGI_PA_ALPHA2,		/* alpha value channel 2		*/
	GGI_PA_ALPHA3,		/* alpha value channel 3		*/
	GGI_PA_BLEND_KEY,	/* a key used for blending		*/
	GGI_PA_BLEND_LOW1,	/* keying low value channel 1		*/
	GGI_PA_BLEND_HIGH1,	/* keying high value channel 1		*/
	GGI_PA_BLEND_LOW2,	/* keying low value channel 2		*/
	GGI_PA_BLEND_HIGH2,	/* keying high value channel 2		*/
	GGI_PA_BLEND_LOW3,	/* keying low value channel 3		*/
	GGI_PA_BLEND_HIGH3,	/* keying high value channel 3		*/

	__GGI_LAST_PA		/* the last defined, must(!) be less 32	*/

} ggi_pixel_attribute;

typedef enum ggi_pixel_attribute_mask {

	GGI_PAM_PRIVATE		= GGI_MASK(GGI_PA_PRIVATE),
	GGI_PAM_APPLICATION	= GGI_MASK(GGI_PA_APPLICATION),
	GGI_PAM_COLOR1		= GGI_MASK(GGI_PA_COLOR1),
	GGI_PAM_COLOR2		= GGI_MASK(GGI_PA_COLOR2),
	GGI_PAM_COLOR3		= GGI_MASK(GGI_PA_COLOR3),
	GGI_PAM_ALPHA		= GGI_MASK(GGI_PA_ALPHA),
	GGI_PAM_INDEX		= GGI_MASK(GGI_PA_INDEX),

	GGI_PAM_FOREGROUND_INDEX= GGI_MASK(GGI_PA_FOREGROUND_INDEX),
	GGI_PAM_TEXTURE_INDEX	= GGI_MASK(GGI_PA_TEXTURE_INDEX), 
	GGI_PAM_BLINK		= GGI_MASK(GGI_PA_BLINK),

	GGI_PAM_ALPHA1		= GGI_MASK(GGI_PA_ALPHA1),
	GGI_PAM_ALPHA2		= GGI_MASK(GGI_PA_ALPHA2),
	GGI_PAM_ALPHA3 		= GGI_MASK(GGI_PA_ALPHA3),
	GGI_PAM_BLEND_KEY	= GGI_MASK(GGI_PA_BLEND_KEY),
	GGI_PAM_BLEND_LOW1	= GGI_MASK(GGI_PA_BLEND_LOW1),
	GGI_PAM_BLEND_HIGH1	= GGI_MASK(GGI_PA_BLEND_HIGH1),
	GGI_PAM_BLEND_LOW2	= GGI_MASK(GGI_PA_BLEND_LOW2),
	GGI_PAM_BLEND_HIGH2	= GGI_MASK(GGI_PA_BLEND_HIGH2),
	GGI_PAM_BLEND_LOW3	= GGI_MASK(GGI_PA_BLEND_LOW3),
	GGI_PAM_BLEND_HIGH3	= GGI_MASK(GGI_PA_BLEND_HIGH3),

	GGI_PAM_ALL		= (1 << __GGI_LAST_PA) - 1

} ggi_pixel_attribute_mask;

typedef struct ggi_alut
{
	ggi_u	size;
	union {

		ggi_pixel	mask;
		ggi_u		value;

	} val[__GGI_MAX_NR_PIXEL_ATTRIBUTES];

} ggi_alut;

typedef struct ggi_fragment
{
	ggi_s	x,y,z;		/* fragments coordinates	*/
	ggi_u	s,t,r,q;	/* texture coordinates		*/
	ggi_u	lod;		/* level of detail		*/
	ggi_u	fog;		/* fog parameter		*/

	ggi_u	attrib[__GGI_MAX_NR_PIXEL_ATTRIBUTES];

} ggi_fragment;

/*	Buffers
**
*/

enum ggi_buffer_layout {

	GGI_BL_PIXEL_LINEAR_BUFFER,

	__GGI_LAST_BL
};


typedef struct ggi_buffer
{
	/*	administrative;
	*/
	struct ggi_buffer	*next;

	void		*app_private;
	void		*lib_private;
	void		*drv_private;

	/*	access info
	*/
	enum ggi_buffer_layout	layout;

	void		*read;		/* buffer address for reads	*/
	void		*write;		/* buffer address for writes	*/
	ggi_u		page_size;	/* zero for true linear buffers	*/

	ggi_u		access;		/* supported access widths	*/
	ggi_u		align;		/* alignment requirements	*/
	ggi_u		swap;		/* swapping requirements	*/

	ggi_u		write_mask;

	/*	buffer layout
	*/
	ggi_u		bpp;		/* bits per pixel		*/
	ggi_s		size_x, size_y;	/* size of application area	*/
	ggi_s		origin_x, origin_y; /* application area origin	*/
	ggi_s		stride;		/* pixels per row		*/

	ggi_pixel_attribute	left_frame_last_attr[__GGI_MAX_NR_FRAMES];
	ggi_u			*left_frame_attr_mask[__GGI_MAX_NR_FRAMES];

	ggi_pixel_attribute	right_frame_last_attr[__GGI_MAX_NR_FRAMES];
	ggi_u			*right_frame_attr_mask[__GGI_MAX_NR_FRAMES];


/********************************************************************/
#if  0
	struct ggi_image	*img;	/* image this belongs to	*/
	struct ggi_viewport	*dpy;	/* display this belongs to	*/

	struct ggi_pixmap	*parent;/* parent if it is a childmap	*/
/*	struct ggi_pixmap	*next;	 circular list of brothers	*/
	struct ggi_pixmap	*childs;/* entry to child pixmap list	*/
#endif

} ggi_buffer;

/*	Images
**
**	An image is a pixmap plus information how to map the pixel values
**	to pixel attributes plus info about the global image properties.
**	Up to __GGI_MAX_NR_FRAMES frames might be stored per image.
**
**	NOTE: Currently certain image operations are not allowed with
**	images that are displayed. (The <dpy> field has a non-NULL value then.)
*/

typedef enum ggi_image_flags
{
	GGI_IF_FIXED		= 0x00000000,

	GGI_IF_MOVE_X		= 0x00000001,
	GGI_IF_MOVE_Y		= 0x00000002,
	GGI_IF_MOVE		= GGI_IF_MOVE_X | GGI_IF_MOVE_Y,

	GGI_IF_SCALE_X		= 0x00000004,
	GGI_IF_SCALE_Y		= 0x00000008,
	GGI_IF_SCALE		= GGI_IF_SCALE_X | GGI_IF_SCALE_Y,

	GGI_IF_AUXILIARY	= 0x00000000,	/* not visible		*/
	GGI_IF_LEFT		= 0x00000010,	/* image for left eye	*/
	GGI_IF_RIGHT		= 0x00000020,	/* image for right eye	*/
	GGI_IF_MONO		= 0x00000030,	/* image for both eyes	*/
	GGI_IF_EYE_MASK		= 0x00000030

} ggi_image_flags;

typedef enum ggi_image_attributes
{
	GGI_IA_SIZE_X,		/* horiz. image size in viewport dots	*/
	GGI_IA_SIZE_Y,		/* vert.  image size in viewport dots	*/
	GGI_IA_ORIGIN_X,	/* horiz. image origin in viewport dots	*/
	GGI_IA_ORIGIN_Y,	/* vert.  image origin in viewport dots	*/

	__GGI_LAST_IA		/* last image attribute defined		*/

} ggi_image_attributes;


typedef struct __ggi_image
{
	struct __ggi_viewport *vp;	/* viewport this belongs to	*/ 
	struct __ggi_image *next;	/* next in direction to viewer	*/
	struct __ggi_image *stereo;	/* image for the other eye	*/

	ggi_image_flags	flags;		/* various flags		*/
	ggi_u		frames;		/* number of frames		*/
	ggi_u		bufx, bufy;	/* minimal buffer size		*/

	ggi_u		*frame_bpp;	/* bits per frame and pixel	*/
	ggi_u		*common_bpp;	/* common bits per pixel	*/

	ggi_u		attr[__GGI_LAST_IA];

} __ggi_image;

typedef __ggi_image *ggi_image;


/*	Viewports
**
**	A viewport holds several images that are stacked on each other and
**	are overlaid according to the rules specified in the images. It
**	represents the device that converts the signals from a video card
**	into a visible real-world picture.
*/
typedef enum ggi_viewport_type
{
	GGI_VP_MEMORY,			/* invisible auxiliary viewport	*/
	GGI_VP_MONO,			/* visible mono display		*/
	GGI_VP_STEREO,			/* visible stereo display	*/

	__GGI_LAST_VP

} ggi_viewport_type;

typedef enum ggi_viewport_attribute
{
	GGI_VA_SIZE_X,			/* x size of the image in dots	*/
	GGI_VA_SIZE_Y,			/* y size of the image in dots	*/ 
	GGI_VA_DIMENSION_X,		/* x real-world size in mm	*/
	GGI_VA_DIMENSION_Y,		/* y real-world size in mm	*/ 
	GGI_VA_COLORSPACE,		/* signal interpretation	*/
	GGI_VA_REFRESH_RATE,		/* at which changes are seen	*/

	__GGI_LAST_VA

} ggi_viewport_attribute;

typedef struct __ggi_viewport
{
	api_context		ctx;
	struct __ggi_viewport	*next;	/* next viewport		*/

	ggi_viewport_type	type;

	ggi_image		img;	/* left image stack		*/

	ggi_u	attr[__GGI_LAST_VA];	/* viewport attributes		*/

} __ggi_viewport, *ggi_viewport;

/*
**	Flags (All the things that can be enabled/disabled.)
*/
typedef enum ggi_flag
{
	GGI_FL_API_WARNINGS,
	GGI_FL_EXIT_ON_ERROR,

	__GGI_LAST_FL

} ggi_flag;

extern ggi_error ggiEnable(api_context ctx, ggi_flag what);
extern ggi_error ggiDisable(api_context ctx, ggi_flag what);

/*
**	API
*/
extern ggi_error ggiViewport(ggi_viewport *vp, api_context ctx,
	ggi_u dotsx, ggi_u dotsy, ggi_viewport_type type);

extern ggi_error ggiImage(ggi_image *img, ggi_viewport vp,
	ggi_u sizex, ggi_u sizey, ggi_u frames, const char *options);

extern ggi_error ggiImageBuffer(ggi_image img, ggi_u *frame_bpp,
	ggi_u frame_bpp_size, ggi_u *common_bpp, ggi_u common_bpp_size);

extern ggi_error ggiInitViewports(ggi_viewport *vp, ...);
extern ggi_error ggiDoneViewports(ggi_viewport *vp, ...);

/*
**	debugging options.
*/

#ifdef	__LIBRARY__

#	define	__lib_error	__ggiError
#	define	__lib_debug	__ggiDebug
#	define	__lib_notice	__ggiNotice

#	ifdef LIB_DEBUG_ANSI_CPP

		extern void __ggiError(ggi_error errno, const char *fmt,...);
		extern void __ggiDebug(unsigned int level, const char *fmt,...);
		extern void __ggiNotice(const char *fmt, ...);

#	endif

#	ifdef LIB_DEBUG_GNU_CPP

		extern void __ggiError(api_context ctx, const char *file,
			unsigned int line, const char *func,
			ggi_error errno, const char *fmt, ...);

		extern void __ggiDebug(api_context ctx, const char *file,
			unsigned int line, const char *func,
			unsigned int level, const char *fmt, ...);

		extern void __ggiNotice(api_context ctx, const char *file,
			unsigned int line, const char *func,
			const char *fmt, ...);
#	endif

#endif

#endif	/* #ifdef __ggi_ggi_h	*/
