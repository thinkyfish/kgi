/* -----------------------------------------------------------------------------
**	libGGI - KGI target driver
** -----------------------------------------------------------------------------
**
**	$Log: KGI.c,v $
**	Revision 1.3  1998/04/04 22:42:55  degas
**	- headers, logging, system layer change
**	
** -----------------------------------------------------------------------------
*/
#define	MAINTAINER	Steffen_Seeger

#include <stdio.h>

#define	__LIBRARY__	GGI
#define	__GGI_DLL__	KGI

#include <ggi/api.h>
#include <ggi/ggi.h>

extern GGI_DLL_INIT(KGI);
GGI_DLL_INIT(KGI)
{
	LIB_DEBUG(1, "initializing vendor=%s, model=%s, param=%s",
		ctx->vendor, ctx->model, (char *) param);
	return GGI_EOK;
}

extern GGI_DLL_DONE(KGI);
GGI_DLL_DONE(KGI)
{
	LIB_DEBUG(1, "shutting down vendor=%s, model=%s\n",
		ctx->vendor, ctx->model);
}
