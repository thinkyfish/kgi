/* LibKGI 
 * 
 * Copyright (C) 1999, 2000 Jon Taylor
 * 
 * This library is the portable userspace
 * method of accessing KGI functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <asm/semaphore.h>

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

kgi_error_t kgiInit(kgi_context_t *ctx, const char *client, const kgi_version_t *version)
{
	union 
	{
		kgic_mapper_identify_request_t	request;
		kgic_mapper_identify_result_t	result;
	} cb;

	fprintf(stderr, "kgiInit()\n");
	
	if (NULL == ctx) 
	{
		return -KGI_NOMEM;
	}
	
	if ((NULL == client) || (NULL == version)) 
	{
		return -KGI_INVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	ctx->mapper.fd = open("/dev/graphic", O_RDWR);
	if (ctx->mapper.fd < 0) 
	{
		perror("Failed to open /dev/graphic: ");
		return -KGI_INVAL;
	}

	memset(&cb, 0, sizeof(cb));
	strncpy(cb.request.client, client, sizeof(cb.request.client));
	cb.request.client[sizeof(cb.request.client) - 1] = 0;
	cb.request.client_version = *version;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_IDENTIFY, &cb)) 
	{
		perror("Failed to identify to mapper");
		return errno;
	}
	
	printf("Identified to mapper %s-%i.%i.%i-%i\n",
	       cb.result.mapper,
	       cb.result.mapper_version.major,
	       cb.result.mapper_version.minor,
	       cb.result.mapper_version.patch,
	       cb.result.mapper_version.extra);
	
	ctx->mapper.resources = cb.result.resources;
	
	return KGI_EOK;
}

kgi_error_t kgiSetImages(kgi_context_t *ctx, kgi_u_t images)
{
	kgi_error_t temp;
	
	fprintf(stderr, "kgiSetImages()\n");
	
	if ((NULL == ctx) || (ctx->mapper.fd < 0)) 
	{
		return -KGI_INVAL;
	}
	
	temp = ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGES, &images) ? errno : KGI_EOK;
	fprintf(stderr, "temp = %.8x\n", temp);
	return temp;
}

kgi_error_t kgiSetImageMode(kgi_context_t *ctx, kgi_u_t image, const kgi_image_mode_t *mode)
{
	kgi_error_t temp;
	kgic_mapper_set_image_mode_request_t cb;

	fprintf(stderr, "kgiSetImageMode()\n");
	
	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) 
	{
		return -KGI_INVAL;
	}
	
	cb.image = image;
	memcpy(&cb.mode, mode, sizeof(cb.mode));
	
	temp = ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGE_MODE, &cb) ? errno : KGI_EOK;
	fprintf(stderr, "temp = %.8x\n", temp);
	return temp;
}

kgi_error_t kgiGetImageMode(kgi_context_t *ctx, kgi_u_t image, kgi_image_mode_t *mode)
{
	kgi_error_t temp;
	
	union 
	{
		kgic_mapper_get_image_mode_request_t	request;
		kgic_mapper_get_image_mode_result_t	result;
	} cb;

	fprintf(stderr, "kgiGetImageMode()\n");
	
	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) 
	{
		return -KGI_INVAL;
	}

	cb.request.image = image;
	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_GET_IMAGE_MODE, &cb)) 
	{
		return errno;
	}
	
	memcpy(mode, &cb.result, sizeof(*mode));	
//	mode->out = NULL;
	
	return KGI_EOK;
}

kgi_error_t kgiCheckMode(kgi_context_t *ctx)
{
	fprintf(stderr, "kgiCheckMode()\n");
	
	if (NULL == ctx) 
	{
		return -KGI_INVAL;
	}
	
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_CHECK, 0) ? errno : KGI_EOK;
}

kgi_error_t kgiSetMode(kgi_context_t *ctx)
{
	fprintf(stderr, "kgiSetMode()\n");
	
	if (NULL == ctx) 
	{
		return -KGI_INVAL;
	}
	
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_SET, 0) ? errno : KGI_EOK;
}

kgi_error_t kgiUnsetMode(kgi_context_t *ctx)
{
	fprintf(stderr, "kgiUnsetMode()\n");
	
	if (NULL == ctx) 
	{
		return -KGI_INVAL;
	}
	
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_DONE, 0) ? errno : KGI_EOK;
}

void kgiPrintImageMode(kgi_image_mode_t *mode)
{
	fprintf(stderr, "kgiPrintImageMode()\n");
	printf("%ix%i (%ix%i) \n", mode->size.x, mode->size.y, mode->virt.x, mode->virt.y);
}

kgi_error_t kgiPrintResourceInfo(kgi_context_t *ctx, kgi_u_t resource)
{
	union 
	{
		kgic_mapper_resource_info_request_t	request;
		kgic_mapper_resource_info_result_t	result;
	} cb;

	fprintf(stderr, "kgiPrintResourceInfo()\n");
	
	cb.request.resource = resource;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_RESOURCE_INFO, &cb)) 
	{
//		return errno;
		return -KGI_INVAL;
	}

	printf("Resource %i (%s) is ", cb.result.resource, cb.result.name);
	
	switch (cb.result.type & KGI_RT_MASK) 
	{
	case KGI_RT_MMIO:
		printf("MMIO: window %i, size %i, align %.8x, "
		       "access %.8x\n",
		       cb.result.info.mmio.window,
		       cb.result.info.mmio.size,
		       cb.result.info.mmio.align,
		       cb.result.info.mmio.access);
		break;

	case KGI_RT_ACCEL:
		printf("ACCEL: recommended are %i buffers of size %i\n",
		       cb.result.info.accel.buffers,
		       cb.result.info.accel.buffer_size);
		break;

	case KGI_RT_SHMEM:
		printf("SHMEM: (maximum) aperture size %i\n",
		       cb.result.info.shmem.aperture_size);
		break;

	default:
		printf("of unknown type\n");
	}
	
	return KGI_EOK;
}
