#ifndef _LIBKGI_H
#define _LIBKGI_H

#include <kgi/kgi.h>
#include <kgi/graphic.h>

typedef struct
{
	struct
	{
		kgi_s_t	fd;
		kgi_u_t	resources;
	} mapper;

} kgi_context_t;

kgi_error_t kgiInit(kgi_context_t *ctx, const char *client, const kgi_version_t *version);
kgi_error_t kgiSetImages(kgi_context_t *ctx, kgi_u_t images);
kgi_error_t kgiSetImageMode(kgi_context_t *ctx, kgi_u_t image, const kgi_image_mode_t *mode);
kgi_error_t kgiGetImageMode(kgi_context_t *ctx, kgi_u_t image, kgi_image_mode_t *mode);
kgi_error_t kgiCheckMode(kgi_context_t *ctx);
kgi_error_t kgiSetMode(kgi_context_t *ctx);
kgi_error_t kgiUnsetMode(kgi_context_t *ctx);
void kgiPrintImageMode(kgi_image_mode_t *mode);
kgi_error_t kgiPrintResourceInfo(kgi_context_t *ctx, kgi_u_t resource);

#endif /* _LIBKGI_H */
