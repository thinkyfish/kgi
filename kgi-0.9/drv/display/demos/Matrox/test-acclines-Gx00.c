/*
** Enable this if you want to test for the Mystique
** WARNING: Do NOT execute this as-is on a Mystique!
** WARNING: Do NOT execute the Mystique version on a Gx00!
*/
#ifndef MYSTIQUE
#define MYSTIQUE 0
#endif
/*
** Test duration parameter (in s)
*/
#define TEST_DURATION 90

/*
**	This is a little test program to test the accelerator
**	mapping code. Only useful with the Matrox accelerator.
*/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <asm/semaphore.h>

#include <kgi/kgi.h>
#include <kgi/graphic.h>

#include <Matrox/Gx00.h>

typedef struct
{
	struct
	{
		kgi_s_t		fd;
		kgi_u_t		resources;

	} mapper;

} kgi_context_t;

kgi_error_t kgiInit(kgi_context_t *ctx, const char *client,
	const kgi_version_t *version)
{
	union {
		kgic_mapper_identify_request_t	request;
		kgic_mapper_identify_result_t	result;
	} cb;

	if (NULL == ctx) {

		return -KGI_NOMEM;
	}
	if ((NULL == client) || (NULL == version)) {

		return -KGI_INVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	ctx->mapper.fd = open("/dev/graphic", O_RDWR);
	if (ctx->mapper.fd < 0) {

		perror("failed to open /dev/graphic: ");
		return -KGI_INVAL;
	}

	memset(&cb, 0, sizeof(cb));
	strncpy(cb.request.client, client,
		sizeof(cb.request.client));
	cb.request.client[sizeof(cb.request.client) - 1] = 0;
	cb.request.client_version = *version;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_IDENTIFY, &cb)) {

		perror("failed to identify to mapper");
		return errno;
	}
	printf("identified to mapper %s-%i.%i.%i-%i\n",
		cb.result.mapper,
		cb.result.mapper_version.major,
		cb.result.mapper_version.minor,
		cb.result.mapper_version.patch,
		cb.result.mapper_version.extra);
	ctx->mapper.resources =
		cb.result.resources;
}

kgi_error_t kgiSetImages(kgi_context_t *ctx, kgi_u_t images)
{
	if ((NULL == ctx) || (ctx->mapper.fd < 0)) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGES, &images) 
		? errno : KGI_EOK; 
}

kgi_error_t kgiSetImageMode(kgi_context_t *ctx, kgi_u_t image,
	const kgi_image_mode_t *mode)
{
	kgic_mapper_set_image_mode_request_t	cb;

	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	cb.image = image;
	memcpy(&cb.mode, mode, sizeof(cb.mode));
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGE_MODE, &cb)
		? errno : KGI_EOK;
}

kgi_error_t kgiGetImageMode(kgi_context_t *ctx, kgi_u_t image,
	kgi_image_mode_t *mode)
{
	union {
		kgic_mapper_get_image_mode_request_t	request;
		kgic_mapper_get_image_mode_result_t	result;
	} cb;

	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	cb.request.image = image;
	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_GET_IMAGE_MODE, &cb)) {

		return errno;
	}
	memcpy(mode, &cb.result, sizeof(*mode));
	mode->out = NULL;
	return KGI_EOK;
}

kgi_error_t kgiCheckMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_CHECK, 0)
		? errno : KGI_EOK;
}

kgi_error_t kgiSetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_SET, 0)
		? errno : KGI_EOK;
}

kgi_error_t kgiUnsetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_DONE, 0)
		? errno : KGI_EOK;
}

void kgiPrintImageMode(kgi_image_mode_t *mode)
{
	printf("%ix%i (%ix%i) \n", mode->size.x, mode->size.y, 
		mode->virt.x, mode->virt.y);
}

kgi_error_t kgiPrintResourceInfo(kgi_context_t *ctx, kgi_u_t resource)
{
	union {
		kgic_mapper_resource_info_request_t	request;
		kgic_mapper_resource_info_result_t	result;
	} cb;

	cb.request.resource = resource;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_RESOURCE_INFO, &cb)) {

		return errno;
	}

	printf("resource %i (%s) is ", cb.result.resource, cb.result.name);
	switch (cb.result.type & KGI_RT_MASK) {

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

struct
{
  kgi_u32_t tag;
  kgi_u32_t dma1;
  kgi_u32_t pitch;
  kgi_u32_t dstorg;
  kgi_u32_t maccess;
  kgi_u32_t cxbndry;
  kgi_u32_t dma2;
  kgi_u32_t ytop;
  kgi_u32_t ybot;
  kgi_u32_t plnwt;
  kgi_u32_t zorg;
} init_dma_cmd;

struct
{
  kgi_u32_t dma1;
  kgi_u32_t xystrt;
  kgi_u32_t xyend;
  kgi_u32_t fcol;
  kgi_u32_t dwgctl; 
} oneline_dma_cmd;

kgi_u_t build_one_buffer(kgi_u8_t* buf, kgi_u_t buf_size,
			 kgi_u_t xmin,kgi_u_t xmax,kgi_u_t ymin,kgi_u_t ymax,
			 kgi_u_t* nline)
{
  kgi_u_t cmdsize = sizeof(oneline_dma_cmd);
  kgi_u_t remaining = buf_size;

  if (remaining < sizeof(init_dma_cmd)) return;

  memcpy(buf,&init_dma_cmd,sizeof(init_dma_cmd));
  buf += sizeof(init_dma_cmd);
  remaining -= sizeof(init_dma_cmd);

  while (remaining > cmdsize)
    {
      kgi_u_t top,left,bottom,right;
      kgi_u32_t color;
      left = xmin + (rand() % (xmax - xmin));
      right = left + (rand() % (xmax - left + 1)) + 1;
      top = ymin + (rand() % (ymax - ymin));
      bottom = top + (rand() % (ymax - top + 1)) + 1;
      color = (random() & 0xFFFF);
      color |= (color << 16);

      oneline_dma_cmd.fcol = color;
      oneline_dma_cmd.xystrt = left | (top << 16);
      oneline_dma_cmd.xyend = right | (bottom << 16);

      memcpy(buf,&oneline_dma_cmd,cmdsize);
      buf += cmdsize;
      remaining -= cmdsize;
      (*nline)++;
    }
  return (buf_size - remaining);
}

/*
** DMA buffer size (do *not* change for the moment)
*/
#define BUFFER_SIZE (4 * 1024)

int main(int argc, char *argv[])
{
	kgi_context_t ctx;
	kgi_version_t	testKGI_version = { 0, 0, 1, 0 };
	kgi_image_mode_t mode;
	kgi_u_t	i, x, y;
	kgi_u16_t *fb;
	kgi_u8_t *accel, *ping, *pong;
	kgi_u_t no = 0;
	struct timeval end_time;
	struct timeval current_time;
	float actual_duration;

	kgiInit(&ctx, "testKGI", &testKGI_version);

	kgiSetImages(&ctx, 1);

	memset(&mode, 0, sizeof(mode));
	mode.fam |= KGI_AM_COLORS;
	mode.bpfa[0] = 5;
	mode.bpfa[1] = 6;
	mode.bpfa[2] = 5;
	mode.size.x = mode.virt.x = 1024;
	mode.size.y = mode.virt.y = 768;
	kgiSetImageMode(&ctx, 0, &mode);

	kgiCheckMode(&ctx);

	kgiGetImageMode(&ctx, 0, &mode);
	kgiPrintImageMode(&mode);

	i = 0;
	while (kgiPrintResourceInfo(&ctx, i) == KGI_EOK) {

		i++;
	}

	kgiSetMode(&ctx);

	/*
	** First, puts a few pixels on the framebuffer directly
	**/
	fb = mmap(NULL, (2 * 1024 * 1024),
		  PROT_READ | PROT_WRITE, MAP_SHARED,
		  ctx.mapper.fd, 
		  GRAPH_MMAP_TYPE_MMIO | (0 << GRAPH_MMAP_RESOURCE_SHIFT));

	memset(fb, 0xff, 0x10000);

	{
	  int h, hend = 700 + 100 * (random() & 15);

	  for (y = 50; y < 400; y++) 
	    for (x = 20; x < 250; x++) {
	      
	      fb[y*mode.virt.x + x] =
		0x7000 | (y * 16) | ((x + i) & 15);
	      for (h = 0; h < hend; h++);
	    }
	}

	/*
	** Then, we map the accelerator
	*/
	accel = mmap(NULL, (2 * BUFFER_SIZE),
		     PROT_READ | PROT_WRITE, MAP_SHARED,
		     ctx.mapper.fd,
		     GRAPH_MMAP_TYPE_ACCEL
		     | 0x00001000
		     | (2 << GRAPH_MMAP_RESOURCE_SHIFT));
	/* We have two buffers */
	ping = accel;
	pong = accel + BUFFER_SIZE;

	/*
	** Now we try several per buffer a hundred times
	*/
	/* First inits the cmd structs */
	init_dma_cmd.tag = MGAG_ACCEL_TAG_DRAWING_ENGINE;
#if MYSTIQUE
	init_dma_cmd.dma1 = 0x20012523;
#else
	init_dma_cmd.dma1 = 0x2001AE23;
#endif
	init_dma_cmd.pitch = 0x00000400; /* PITCH: ylin=0, iy=1024 */
	init_dma_cmd.dstorg = 0x00000000; /* DSTORG */
	init_dma_cmd.maccess = 0x40000001; /* MACCESS */
	init_dma_cmd.cxbndry = 0x0FFF0000, /* CXBNDRY */
	init_dma_cmd.dma2 = 0x03072726;
	init_dma_cmd.ytop = 0x00000000; /* YTOP */
	init_dma_cmd.ybot = 0x00FFFFFF; /* YBOT */
	init_dma_cmd.plnwt = 0xFFFFFFFF; /* PLNWT */
	init_dma_cmd.zorg = 0x00080000, /* ZORG (not used normally)*/
	oneline_dma_cmd.dma1 = 0x40091110;
	oneline_dma_cmd.dwgctl = 0x840C4813; /* DWGCTL (via 0x1D00) */

	gettimeofday(&end_time,NULL);
	end_time.tv_sec += TEST_DURATION;

	do
	{
	  kgi_u_t size;
	  size = build_one_buffer(ping, BUFFER_SIZE,
				  20,1004,20,748,
				  &no);
	  (*(pong + size)) = 0x15; /* Tells size */

	  size = build_one_buffer(pong, BUFFER_SIZE,
				  20,1004,20,748,
				  &no);
	  (*(ping + size)) = 0x15; /* Tells size */

	  gettimeofday(&current_time,NULL);
	}
	while (timercmp(&current_time,&end_time,<));

	actual_duration =
	  (current_time.tv_sec - end_time.tv_sec + TEST_DURATION)
	  + 1e-6 * (current_time.tv_usec - end_time.tv_usec);

	printf("%i lines in %f seconds, i.e. %f line/s\n",
	       no, actual_duration, ((float)no)/actual_duration);

	return 0;
}
