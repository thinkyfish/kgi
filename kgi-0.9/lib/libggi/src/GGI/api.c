/* -----------------------------------------------------------------------------
**	libGGI - API driver
** -----------------------------------------------------------------------------
**
**	$Log: api.c,v $
**	Revision 1.4  1998/04/18 16:34:20  seeger_s
**	- fixes to new E(subsystem,code) macro
**	
**	Revision 1.3  1998/04/04 22:42:55  degas
**	- headers, logging, system layer change
**	
** -----------------------------------------------------------------------------
*/
#define	MAINTAINER	Steffen_Seeger

#include <stdlib.h>
#include <string.h>

#define	__LIBRARY__	GGI
#define	__GGI_DLL__	api

#include <ggi/api.h>
#include <ggi/ggi.h>

extern GGI_DLL_INIT(api);
GGI_DLL_INIT(api)
{
	char	*dpy_id	 = (char *) param;
	char drv[256];
	ggi_u	i;

	if ((NULL == dpy_id) || (*dpy_id == 0)) {

		const char *vars[] = { "GGI_DISPLAY", "DISPLAY", NULL };
		ggi_u	i = 0;
		char *env;

		do {

			env = getenv(vars[i++]);

		} while (((env == NULL) || (*env == 0)) && (vars[i] != NULL));

		if (env) {

			return __ggiInitDLL_api(ctx, env);

		} else {

			return __ggiInitDLL_api(ctx, "KGI@/dev/graphic");
		}
	}

	LIB_DEBUG(1, "display target is '%s'", dpy_id);

	i = 0;
	while (dpy_id[i] && (dpy_id[i] != '@') && (i < 255)) {

		drv[i] = dpy_id[i];
		i++;
	}
	drv[i] = 0;

	if ('@' == dpy_id[i++]) {

		LIB_DEBUG(1,"loading %s driver, parameter %s", drv, dpy_id + i);
		ctx->drv.priv_ptr =
			(void *) __ggiDLLOpen("GGI", drv, dpy_id + i);

	} else {

		LIB_DEBUG(1,"loading X11 driver, parameter %s", dpy_id);
		ctx->drv.priv_ptr = (void *) __ggiDLLOpen("GGI", "X11", dpy_id);
	}

	return (ctx->drv.priv_ptr == NULL) ? E(LIB, FAILED) : GGI_EOK;
}

extern GGI_DLL_DONE(api);
GGI_DLL_DONE(api)
{
#	define	drv_ctx	((api_context) ctx->drv.priv_ptr)

	if (drv_ctx) {

		__ggiDLLClose(drv_ctx);
		ctx->drv.priv_ptr = NULL;
	}

#	undef	drv_ctx
}
