/*
** The following defines should help you compile demo programs
** for several generations of Matrox chipsets.
**  - 2D accel demos (box, lines) should work as-is on all chipsets
** independently of the conditional compilation (there is a minor
** difference [DSTORG vs YDSTORG] but the driver translates the register
** access);
**  - 3D accel demos only work on the Gx00 (the Mystique has no WARP setup
** engine): and the G200 and the G400 uses different pipes so they require
** different texture setup. One *requires* the conditional compilation
** to see correct demos.
**
** Executing the bad version on some chipset should not lock up your
** computer (in-driver validation occurs in the latest version of the
** KGI Matrox driver) - but there is no guarantee!! (Be careful not to
** try this then...)
*/

#ifndef CHIPSET_G200
#ifndef CHIPSET_G400
#ifndef CHIPSET_MYSTIQUE
/* One of those should be set to 1 before including mga_accel.h */
#define CHIPSET_G200 1
//#define CHIPSET_G400 1
//#define CHIPSET_MYSTIQUE 1
#endif
#endif
#endif

/*
** Test duration parameter (in s)
*/
#define TEST_DURATION 10

#define BUFFER_IN_MEMORY 0

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
#include "mga_accel.h"

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
  kgi_u8_t *accel, *ping, *pong, *warp, *wping, *wpong, *ptr;
  kgi_u_t no = 0;
  struct timeval end_time;
  struct timeval current_time;
  float actual_duration;
  char buffer_test[2*BUFFER_SIZE];

  mga_dma_ring_t my_ring;
  mga_accel_state_t my_state;

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
  fb = mmap(NULL, (4 * 1024 * 1024),
	    PROT_READ | PROT_WRITE, MAP_SHARED,
	    ctx.mapper.fd, 
	    GRAPH_MMAP_TYPE_MMIO | (0 << GRAPH_MMAP_RESOURCE_SHIFT));

  memset(fb, 0xff, 0x10000);

  {
    int h, hend = 7 + 2 * (random() & 15);

    for (y = 0; y < 768; y++) 
      for (x = 0; x < 1024; x++) {
	      
	fb[y*mode.virt.x + x] =
	  // 0x7000 | (y * 16) | ((x + i) & 15);
	  0 + x & 777;
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
  if (((int)accel) <= 0) {
    printf("ACCEL mmap() failed (ret = %.8x)\n", accel);
    exit(1);
  } else {
    printf("ACCEL mmap() succeeded (ret = %.8x)\n", accel);
  }

#if BUFFER_IN_MEMORY
  mga_dma_init(&my_ring, buffer_test, BUFFER_SIZE, 2);
#else
  mga_dma_init(&my_ring, accel, BUFFER_SIZE, 2);
#endif
  mga_accel_init(&my_state, &my_ring, &mode);

  gettimeofday(&end_time,NULL);
  end_time.tv_sec += TEST_DURATION;

  do
    {
      {
	int xmin = 50;
	int xmax = 1000;
	int ymin = 50;
	int ymax = 750;

	int top,left,width,height;
	mga_vertex_color_t color;

	left = xmin + (rand() % (xmax - xmin));
	width = (rand() % (xmax - left + 1)) + 1;
	top = ymin + (rand() % (ymax - ymin));
	height = (rand() % (ymax - top + 1)) + 1;
	color.blue = (random() & 0xFF);
	color.green = (random() & 0xFF);
	color.red = (random() & 0xFF);
	color.alpha = (random() & 0xFF);
#if 0
	printf("left=%i,right=%i,top=%i,bottom=%i "
	       "color=RGBA(%.2x,%.2x,%.2x,%.2x)\n",
	       left,right,top,bottom,color.red,color.green,color.blue,
	       color.alpha);
#endif
	mga_accel_set_foreground(&my_state, color);
	mga_accel_draw_box(&my_state, left, top, width, height);

#if 0 /* run once */
	mga_accel_flush(&my_state);
	exit();
#endif

	no++;
      }

      gettimeofday(&current_time,NULL);
    }
  while (timercmp(&current_time,&end_time,<));

  mga_accel_flush(&my_state);

  gettimeofday(&current_time,NULL);
	   
  actual_duration =
    (current_time.tv_sec - end_time.tv_sec + TEST_DURATION)
    + 1e-6 * (current_time.tv_usec - end_time.tv_usec);
	 
  printf("%i boxes in %f seconds, i.e. %f box/s\n",
	 no, actual_duration, ((float)no)/actual_duration);
	
  return 0;
}

