/* -----------------------------------------------------------------------------
**	GGI generic API handling / DLL loader code
** -----------------------------------------------------------------------------
**
**	$Log: dll.c,v $
**	Revision 1.4  1998/04/18 16:34:19  seeger_s
**	- fixes to new E(subsystem,code) macro
**	
**	Revision 1.3  1998/04/04 22:42:55  degas
**	- headers, logging, system layer change
**	
** -----------------------------------------------------------------------------
*/
#define	MAINTAINER	Steffen_Seeger

#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#define	__LIBRARY__

#include <ggi/api.h>
#include <ggi/ggi.h>

/*
**	dynamic loader and linker interface
*/

#define	__dlopen	dlopen
#define	__dlclose	dlclose
#define	__dlerror	dlerror
#define	__dlsym		dlsym

#define	APPEND(s)	do { strcpy(foo, s); foo += strlen(s); } while (0)



api_context __ggiDLLOpen(const char *lib, const char *api, void *param)
/*	Load a dynamic library, allocate the context structure and do the
**	library initialization.
** NOTE	The returned context **MUST**NOT** be reallocated, as other libraries
** NOTE	might have stored references to it.
*/
{
	__api_context	*ctx = NULL;
	char	name[256], *foo;
	int	err;
	ggi_dll_init_fn	*Init;
	ggi_dll_done_fn	*Done;

	if (lib == NULL || api == NULL) {

		LIB_ERROR(E(LIB, INVAL), "invalid parameters");
		return NULL;
	}
	LIB_ASSERT(strlen(lib) < sizeof(ctx->vendor));
	LIB_ASSERT(strlen(api) < sizeof(ctx->model));

	/*	allocate dynamic stuff
	*/
	ctx = malloc(sizeof(__api_context));
	if ((NULL == ctx) || (strlen(lib) + strlen(api) + 4 > sizeof(name))) {

		free(ctx);
		LIB_ERROR(E(LIB, NOMEM), "out of memory");
		return NULL;
	}
	memset((void *) ctx, 0, sizeof(__api_context));
	strcpy(ctx->vendor, lib);
	strcpy(ctx->model, api);

	foo = name;
	APPEND(lib);
	APPEND("/");
	APPEND(api);
	APPEND(".so");
	LIB_DEBUG(2, "loading %s", name);
	ctx->dll.priv_ptr = __dlopen(name, (strcmp(api, "api") == 0)
		? (RTLD_LAZY | RTLD_GLOBAL) : RTLD_LAZY);

	if (NULL == ctx->dll.priv_ptr) {

		free(ctx);
		LIB_ERROR(E(LIB, DLL_ERROR), __dlerror());
		return NULL;
	}

	foo = name;
	APPEND(GGI_DLL_INIT_PREFIX);
	APPEND(api);
	Init = (ggi_dll_init_fn *) __dlsym(ctx->dll.priv_ptr, name);
	LIB_DEBUG(2, "%s = %p", name, Init);

	foo = name;
	APPEND(GGI_DLL_DONE_PREFIX);
	APPEND(api);
	Done = (ggi_dll_done_fn *) __dlsym(ctx->dll.priv_ptr, name);
	LIB_DEBUG(2, "%s = %p", name, Done);

	if (NULL == Init || NULL == Done) {

		__dlclose(ctx->dll.priv_ptr);
		free(ctx);
		LIB_ERROR(E(LIB, FAILED), __dlerror());
		return NULL;
	}

	err = Init(ctx, param);
	LIB_DEBUG(2, "err = %i", err);
	if (GGI_EOK != err) {

		__dlclose(ctx->dll.priv_ptr);
		free(ctx);
		LIB_ERROR(E(LIB, FAILED),
			"Initialization of %s/%s.so failed.", lib, api);
		return NULL;
	}

	ctx->refcnt++;
	return ctx;
}



api_context __ggiDLLReference(api_context ctx)
/*	Get reference to ctx. This is just to allow us to do bookkeeping
**	and disallocate the context in __ggiDLLDone() properly.
*/
{
	if (ctx) {

		LIB_DEBUG(2, "referenced %s/%s.so", ctx->vendor, ctx->model);
		ctx->refcnt++;
	}
	return ctx;
}



void __ggiDLLClose(api_context ctx)
/*	If there no references left to this context, unload the library and
**	deinitialize.
*/
{
	ggi_dll_done_fn	*Done;
	char	name[160], *foo;

	LIB_ASSERT(ctx);
	LIB_ASSERT(ctx->dll.priv_ptr);

	if (ctx->refcnt--) {

		LIB_DEBUG(2, "dereferencing %s/%s.so", ctx->vendor, ctx->model);
		return;
	}
	LIB_DEBUG(2, "unloading %s/%s.so", ctx->vendor, ctx->model);

	foo = name;
	APPEND(GGI_DLL_DONE_PREFIX);
	APPEND(ctx->model);
	Done = (ggi_dll_done_fn *) __dlsym(ctx->dll.priv_ptr, name);
	LIB_DEBUG(2, "%s = %p", name, Done);
	
	Done(ctx);
	__dlclose(ctx->dll.priv_ptr);
	memset((void *) ctx, 0, sizeof(__api_context));
	free(ctx);
}
