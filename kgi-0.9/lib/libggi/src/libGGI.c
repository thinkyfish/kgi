/* -----------------------------------------------------------------------------
**	libGGI main stub/implemenation
** -----------------------------------------------------------------------------
**
**	$Log: libGGI.c,v $
**	Revision 1.4  1998/04/18 16:34:19  seeger_s
**	- fixes to new E(subsystem,code) macro
**	
**	Revision 1.3  1998/04/04 22:42:55  degas
**	- headers, logging, system layer change
**	
** -----------------------------------------------------------------------------
*/
#define	MAINTAINER	Steffen_Seeger

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define	__LIBRARY__	1

#include <ggi/api.h>
#include <ggi/ggi.h>

#ifdef	GGI_CONFIG_USE_THREADS
#	include <pthread.h>
#	define	MUTEX_STATIC(mutex)	static pthread_mutex_t	mutex = NULL;
#	define	MUTEX_ENTER(mutex)	pthread_mutex
#else
#	define	MUTEX_INIT(mutex)
#	define	MUTEX_ENTER(mutex)
#	define	MUTEX_LEAVE(mutex)
#endif

/*	These functions do not belong to a library context. Therefore we have
**	care about global threadsafety.
*/
#ifdef	GGI_CONFIG_THREADS

	enum __ggi_global
	{
		__GGI_API_INIT,
		__GGI_API_DONE,
		__GGI_API_VIEWPORT,
		__GGI_API_IMAGE,
		__GGI_API_IMAGEBUFFER,

		__GGI_LAST_GLOBAL
	};

	static ggi_u __ggi_initialized = 0;
	static MUTEX_T __ggi_mutex[__GGI_LAST_GLOBAL];

#	define	GLOBAL_ENTER(func)	MUTEX_ENTER(__ggi_mutex[func])
#	define	GLOBAL_LEAVE(func)	MUTEX_LEAVE(__ggi_mutex[func])

#else

#	define	GLOBAL_ENTER(func)
#	define	GLOBAL_LEAVE(func)

#	define	__ggi_initialized	1
#	define	__GGI_LAST_GLOBAL	1

#endif

int ggiAPIInit(api_context *ctx, const char *api, void *param)
/*	Initialize render-API context. This is usually the first function
**	you call in your application. To be really safe, we allow only one
**	thread to execute this function.
*/
{
	if (ctx == NULL) {

		return -E(LIB, NOMEM);
	}

	if (! __ggi_initialized) {

		ggi_u	i;
		for (i = 0; i < __GGI_LAST_GLOBAL; i++) {

			MUTEX_INIT(__ggi_mutex[i]);
		}
	}

	GLOBAL_ENTER(GGI_API_INIT);

		*ctx = __ggiDLLOpen(api, "api", param);

	GLOBAL_LEAVE(GGI_API_INIT);
	return (NULL == *ctx) ? E(LIB, FAILED) : GGI_EOK;
}

void ggiAPIDone(api_context *ctxp)
/*	Deinitialize a render-API context. This is usually the last function
**	you call in a API life time.
*/
{
#	define ctx	(*ctxp)
	LIB_ASSERT(__ggi_initialized);

	GLOBAL_ENTER(GGI_API_DONE);

		__ggiDLLClose(ctx);
		ctx = NULL;

	GLOBAL_LEAVE(GGI_API_DONE);
#	undef	ctx
}



#define	memclr(x)	memset(x, 0, sizeof(*(x)));

/*	This creates a new viewport with a resolution of <sizex> times
**	<sizey> dots. <type> indicates if a mono or stereo viewport
**	is required. Mono viewports have only left-eye images.
*/
ggi_error ggiViewport(ggi_viewport *vp, api_context ctx,
	ggi_u sizex, ggi_u sizey, ggi_viewport_type type)
{
	ggi_viewport new_vp;

	if ((NULL == vp) || (NULL == ctx) || (__GGI_LAST_VP <= type)) {

		return E(LIB, INVAL);
	}

	new_vp = malloc(sizeof(*new_vp));
	if (NULL == new_vp) {

		*vp = NULL;
		LIB_ERROR (E(LIB, NOMEM), "out of memory");
		return E(LIB, NOMEM);
	}
	memclr(new_vp);

	MUTEX_ENTER(ctx->mutex);

		new_vp->ctx = ctx;
		new_vp->next = (ggi_viewport) ctx->lib.priv_ptr;
		new_vp->type = type;
		new_vp->attr[GGI_VA_SIZE_X] = sizex;
		new_vp->attr[GGI_VA_SIZE_Y] = sizey;
		*vp = new_vp;
		ctx->lib.priv_ptr = (void *) new_vp;

	MUTEX_LEAVE(ctx->mutex);

	return GGI_EOK;
}

/*	Add a image to a viewport <vp>.
**	The buffers should hold at least <bufx> times <bufy> pixels
**	and <frames> frames. The actual images may be (independently)
**	scalable and movable as indicated by <flags>.
**	A handles are returned in <*left> and <*right> if result is GGI_EOK.
**	<options> is a string of options, that may contain the following
**	option keywords:
**		fixed	not moveable or scaleable (default if no options given)
**		scalex	scaleable in x direction
**		scaley	scaleable in y direction
**		scale	same as "scalex, scaley"
**		movex	moveable in x direction
**		movey	moveable in y direction
**		move	same as "movex, movey"
**		mono	image is a mono image (default if no options given)
**		stereo	image is a stereo image
*/
ggi_error ggiImage(ggi_image *img, ggi_viewport vp,
	ggi_u bufx, ggi_u bufy, ggi_u frames, const char *options)
{
	api_context ctx;
	ggi_image new_left, new_right;
	ggi_image_flags flags = GGI_IF_FIXED;
	ggi_u	stereo = 0;

	if ((NULL == img) || (NULL == vp)) {

		return E(LIB, INVAL);
	}
	ctx = vp->ctx;

	if (options) {

		const char *foo;

		foo = options;
		while ((foo = strstr(foo, "scale"))) {

			flags |= (foo[5] == 'x') ? GGI_IF_SCALE_X : 0;
			flags |= (foo[5] == 'y') ? GGI_IF_SCALE_Y : 0;
			flags |= (foo[5] != 'x') && (foo[5] != 'y')
				? GGI_IF_SCALE : 0;
			foo++;
		}

		foo = options;
		while ((foo = strstr(foo, "move"))) {

			flags |= (foo[4] == 'x') ? GGI_IF_MOVE_X : 0;
			flags |= (foo[4] == 'y') ? GGI_IF_MOVE_Y : 0;
			flags |= (foo[4] != 'x') && (foo[4] != 'y')
				? GGI_IF_MOVE : 0;
			foo++;
		}

		stereo = (NULL != strstr(options, "stereo"));
	}

	new_left  = malloc(sizeof(*new_left));
	new_right = stereo ? malloc(sizeof(*new_right)) : NULL;
	if ((NULL == new_left) || (stereo && (NULL == new_right))) {

		free(new_left);
		free(new_right);
		LIB_ERROR (E(LIB, NOMEM), "out of memory");
		return  E(LIB, NOMEM);
	}

	GLOBAL_ENTER(__GGI_API_IMAGE);

		if (stereo) {

			memclr(new_right);
			new_right->vp = vp;
			new_right->next	= vp->img;
			new_right->stereo = new_left;
			new_right->flags = flags | GGI_IF_RIGHT;
			new_right->frames = frames;
			new_right->bufx	= bufx;
			new_right->bufy	= bufy;

		} else {

			flags |= GGI_IF_MONO;
		}

		memclr(new_left);
		new_left->vp = vp;
		new_left->next = vp->img;
		new_left->stereo = new_right;
		new_left->flags = flags | GGI_IF_LEFT;
		new_left->frames = frames;
		new_left->bufx = bufx;
		new_left->bufy = bufy;

		*img = new_left;
		vp->img = new_left;

	GLOBAL_LEAVE(__GGI_API_IMAGE);

	return GGI_EOK;
}

/*	Add buffer requirements to an image. If the image is part of a stereo
**	image, requirements apply to both left and right eye images.
**	Per attribute <attr> and pixel, at least <frame_bpp[attr]> bits have
**	to be stored per frame and at least <common_bpp[attr]> bits have
**	to be stored common to all frames.
*/
ggi_error ggiImageBuffer(ggi_image img, ggi_u *frame_bpp, ggi_u frame_bpp_size,
	ggi_u *common_bpp, ggi_u common_bpp_size)
{
#	define	SIZE	__GGI_LAST_PA*sizeof(ggi_u)

	ggi_u	*common, *frame;

	if (NULL == img) {

		return E(LIB, INVAL);
	}

	common = img->common_bpp ? img->common_bpp : malloc(SIZE);
	frame  = img->frame_bpp  ? img->frame_bpp  : malloc(SIZE);
	if (! (common && frame)) {

		return E(LIB, NOMEM);
	}

	memset(common, 0, SIZE);
	if (common_bpp && common_bpp_size) {

		memcpy(common, common_bpp, common_bpp_size*sizeof(ggi_u));
	}
	memset(frame,  0, SIZE);
	if (frame_bpp && frame_bpp_size) {

		memcpy(frame, frame_bpp, frame_bpp_size*sizeof(ggi_u));
	}

	img->common_bpp = common;
	img->frame_bpp  = frame;
	if (img->stereo) {

		img->stereo->common_bpp = common;
		img->stereo->frame_bpp  = frame;

	}

	return GGI_EOK;

#	undef	SIZE
}


ggi_error ggiInitViewports(ggi_viewport *vp, ...)
{
	/*	... init hardware ...
	*/
	*vp = *vp;
	return E(LIB, NOT_IMPLEMENTED);
}

ggi_error ggiDoneViewports(ggi_viewport *vp, ...)
{
	/*	... shutdown hardware ...
	**	... update context struct ...
	*/
	*vp = *vp;
	return E(LIB, NOT_IMPLEMENTED);
}

/*
**	Error reporting.
*/
#if (HOST_OS == HOST_OS_BeOS)
#	define	vsnprintf(buf, size, fmt, arg)	vsprintf(buf, fmt, arg)
#endif

#ifdef	LIB_DEBUG_ANSI_CPP

void __ggiError(ggi_error errno, const char *fmt, ...)
{
	char msg[256];
	va_list arg;

	LIB_TRACE(0, memset(msg, 0, sizeof(msg)));

	va_start(arg, fmt);
		vsnprintf(msg, sizeof(msg), fmt, arg);
	va_end(arg);

	LIB_ASSERT(msg[255] == 0);

	fprintf(stderr, "error %.8x: %s\n", errno, msg);
}

void __ggiDebug(unsigned int level, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
		fprintf(stderr, "debug %i: ", level);
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, "\n");
	va_end(arg);
}

void __ggiNotice(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
		vfprintf(stderr, fmt, arg);
	va_end(arg);
}

#endif	/* #ifdef LIB_DEBUG_ANSI_CPP */


#ifdef LIB_DEBUG_GNU_CPP
void __ggiError(api_context ctx, const char *file, unsigned int line,
	const char *func, ggi_error errno, const char *fmt, ...)
{
	va_list arg;

	LIB_TRACE(0, memset(ctx->err.msg, 0, sizeof(ctx->err.msg)));

	strncpy(ctx->err.file, file, sizeof(ctx->err.file));
	strncpy(ctx->err.func, func, sizeof(ctx->err.func));
	ctx->err.line = line;
	ctx->err.errno = errno;

	va_start(arg, fmt);
		vsnprintf(ctx->err.msg, sizeof(ctx->err.msg), fmt, arg);
	va_end(arg);

	LIB_ASSERT(ctx->err.msg[255] == 0);

	fprintf(stderr, "%s:%i:%s:error %.8x: %s\n",
		ctx->err.file, ctx->err.line, ctx->err.func, ctx->err.errno,
		ctx->err.msg);
}


void __ggiDebug(api_context ctx, const char *file, unsigned int line,
	const char *func, unsigned int level, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
		fprintf(stderr, "%s:%s:%i:%s:debug %i: ",
			ctx->vendor, file, line, func, level);
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, "\n");
	va_end(arg);
}


void __ggiNotice(api_context ctx, const char *file, unsigned int line,
	const char *func, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
		if (ctx && file && line && func) {

			/*	fool gcc;
			*/
		}
		vfprintf(stderr, fmt, arg);
	va_end(arg);
}
#endif	/* #ifdef LIB_DEBUG_GNU_CPP */
