/* ----------------------------------------------------------------------------
**	GGI generic library interface and DLL API
** ----------------------------------------------------------------------------
**
**	Copyright (C) 1997-1998	Steffen Seeger
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: api.h,v $
**	Revision 1.3  1998/04/04 22:42:54  degas
**	- headers, logging, system layer change
**	
** ----------------------------------------------------------------------------
*/

#ifndef	_ggi_api_h
#define	_ggi_api_h

/*
**	library context
*/
#include <ggi/system.h>
#include <ggi/errno.h>

#ifdef __LIBRARY__

typedef struct __api_context {

	__ggi_sys_private	app;	/* application private data	*/
	__ggi_sys_private	lib;	/* (well defined) library state	*/
	__ggi_sys_private	api;	/* api call table		*/
	__ggi_sys_private	drv;	/* driver state			*/
	__ggi_sys_private	dll;	/* dynamic library loader state	*/

	__ggi_sys_ascii	vendor[64];	/* vendor 			*/
	__ggi_sys_ascii	model[64];	/* driver			*/
	__ggi_sys_u	refcnt;		/* reference count		*/

	struct {
		__ggi_sys_ascii	file[124];	/* file of last error	*/
		__ggi_sys_ascii	func[124];	/* function of error	*/
		__ggi_sys_u	line;		/* source line of error	*/
		__ggi_sys_error	errno;		/* error code		*/
		__ggi_sys_ascii	msg[256];	/* error message	*/
	} err;

} __api_context, *api_context;

/*
**	DLL dynamic loader and linker API
*/
typedef	__ggi_sys_error ggi_dll_init_fn(api_context ctx, void *param);
typedef void ggi_dll_done_fn(api_context ctx);

#define	GGI_DLL_INIT_PREFIX	"__ggiInitDLL_"
#define	GGI_DLL_DONE_PREFIX	"__ggiDoneDLL_"


#define	GGI_DLL_INIT(dll_name)	\
	__ggi_sys_error __ggiInitDLL_ ## dll_name (api_context ctx, void *param)
#define	GGI_DLL_DONE(dll_name)	\
	void __ggiDoneDLL_ ## dll_name (api_context ctx)

extern api_context __ggiDLLOpen(const char *lib, const char *drv, void *param);
extern api_context __ggiDLLReference(api_context ctx);
extern void __ggiDLLClose(api_context ctx);

#else	/* #ifdef __LIBRARY__	*/

typedef struct __api_context 
{
	__ggi_sys_private	app;	/* application private		*/
	__ggi_sys_private	lib;	/* (well defined) library state	*/

} *api_context;

#endif	/* #ifdef __LIBRARY__	*/

extern int  ggiAPIInit(api_context *ctxp, const char *api, void *param);
extern void ggiAPIDone(api_context *ctxp);

#endif	/* #ifdef _ggi_api_h */
