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
/* NB: For Marie... R.O. */

#ifndef CHIPSET_G200
#ifndef CHIPSET_G400
#ifndef CHIPSET_MYSTIQUE
/* One of those should be set to 1 before including mga_accel.h */
//#define CHIPSET_G200 1
#define CHIPSET_G400 1
//#define CHIPSET_MYSTIQUE 1
#endif
#endif
#endif

/*
** Test duration parameter (in s)
*/
#define TEST_DURATION 15

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
** DMA buffer size and number
*/
#define BUFFER_SIZE_LN2 0
#define BUFFER_SIZE ((1 << BUFFER_SIZE_LN2) * 4 * 1024)
#define BUFFER_NUMBER_LN2 2
#define BUFFER_NUMBER (1 << BUFFER_NUMBER_LN2)

int main(int argc, char *argv[])
{
  kgi_context_t ctx;
  kgi_version_t	testKGI_version = { 0, 0, 1, 0 };
  kgi_image_mode_t mode;
  kgi_u_t	i, x, y;
  kgi_u16_t *fb;
  kgi_u8_t *accel, *ptr;
  kgi_u_t no = 0;
  struct timeval end_time;
  struct timeval current_time;
  float actual_duration;
  kgi_u32_t warp_tag;
#if 0
  mga_vertex_color_t col1 = { 0, 255, 0, 0 }; /* Green, semi opaque */
  mga_vertex_color_t col2 = { 255,0,0,0 }; /* Blue, semi opaque */
  mga_vertex_color_t col3 = { 0,0,255,0 }; /* Red, semi opaque */
#else
  mga_vertex_color_t col1 = { 196, 196, 196, 0 };
  mga_vertex_color_t col2 = { 110, 110, 110, 0 };
  mga_vertex_color_t col3 = { 240, 240, 240, 0 };
#endif
  mga_vertex_color_t spcol = { 128,192,64,128 }; /* Gray, fog=128 */
  mga_vertex_color_t black = { 0,0,0,0 }; /* Black */
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
  accel = mmap(NULL, BUFFER_SIZE * BUFFER_NUMBER,
	       PROT_READ | PROT_WRITE, MAP_SHARED,
	       ctx.mapper.fd,
	       GRAPH_MMAP_TYPE_ACCEL
	       | (BUFFER_SIZE_LN2 << GRAPH_MMAP_ACCEL_MINSIZE_SHIFT)
	       | (BUFFER_SIZE_LN2 << GRAPH_MMAP_ACCEL_MAXSIZE_SHIFT)
#if 0 /* TODO: !!!!! */
	       | (1 << GRAPH_MMAP_ACCEL_PRIORITY_SHIFT)
#endif
	       | (BUFFER_NUMBER_LN2 << GRAPH_MMAP_ACCEL_BUFFERS_SHIFT)
	       | (2 << GRAPH_MMAP_RESOURCE_SHIFT));
  if (((int)accel) <= 0) {
    printf("ACCEL mmap() failed (ret = %.8x)\n", accel);
    exit(1);
  } else {
    printf("ACCEL mmap() succeeded (ret = %.8x)\n", accel);
  }

#if BUFFER_IN_MEMORY
  mga_dma_init(&my_ring, buffer_test, BUFFER_SIZE, BUFFER_NUMBER);
#else
  mga_dma_init(&my_ring, accel, BUFFER_SIZE, BUFFER_NUMBER);
#endif
  mga_accel_init(&my_state, &my_ring, &mode);

  /*
  ** "Zeroes" the Zbuffer
  */
  {
    kgi_u16_t *ptr = (kgi_u16_t*)((kgi_u_t)fb + my_state.zbuffer);
    printf("Z-buffer offset: %.8x (%f MB)\n",
	   my_state.zbuffer,
	   (float)my_state.zbuffer / (1024.0 * 1024.0));
    for (y = 0; y < mode.virt.y; y++)
      {
	for (x = 0; x < mode.virt.x; x++)
	  {
	    (*ptr) = ~0x0;
	    ptr++;
	  }
      }
  }

  gettimeofday(&end_time,NULL);
  end_time.tv_sec += TEST_DURATION;

  do
    {
      {
	mga_vertex_color_t color;
	mga_vertex_t vert[3];
	mga_vertex_t fixvert[3];

	for (i = 0; i < 3; i++)
	  {
	    kgi_u_t r = (rand() % 3);
	    vert[i].rhw = 1.0;
	    vert[i].tu0 = 0.0;
	    vert[i].tv0 = 0.0;
	    vert[i].x = (float)(rand() % 10000) * 0.1 + 5;
	    vert[i].y = (float)(rand() % 7000)  * 0.1 + 5;
	    vert[i].z = (float)(rand() % 99) * 0.01; // 0.0; // 0.01;
	    //vert[i].color = (r == 0) ? col1 : ((r==1) ? col2 : col3);
	    vert[i].color = (i == 0) ? col1 : ((i==1) ? col2 : col3);
	    vert[i].specular = black;
	    vert[i].specular = spcol; /* strange effect */
	  }
	mga_accel_draw_triangle(&my_state, &(vert[0]), &(vert[1]), &(vert[2]));
	no++;

	for (i = 0; i < 3; i++)
	  {
	    fixvert[i].specular = black;
	  }
	fixvert[0].x = 200.0;
	fixvert[0].y = 250.0;
	fixvert[0].z = 0.001;
	fixvert[0].color = col1;
	fixvert[0].rhw = 1.0;
	fixvert[0].tu0 = 0.0;
	fixvert[0].tv0 = 1.0;

	fixvert[1].x = 600.0;
	fixvert[1].y = 150.0;
	fixvert[1].z = 0.006;
	fixvert[1].color = col2;
	fixvert[1].rhw = 0.5;
	fixvert[1].tu0 = 3.324;
	fixvert[1].tv0 = 0.0;

	fixvert[2].x = 800.0;
	fixvert[2].y = 750.0;
	fixvert[2].z = 0.008;
	fixvert[2].color = col3;
	fixvert[2].rhw = 1.0;
	fixvert[2].tu0 = 2.0;
	fixvert[2].tv0 = 2.0;

	mga_accel_draw_triangle(&my_state,
				&(fixvert[0]), &(fixvert[1]), &(fixvert[2]));
	no++;

#if 0
	mga_accel_flush(&my_state);
	exit(1);
#endif

      }

      gettimeofday(&current_time,NULL);
    }
  while (timercmp(&current_time,&end_time,<));

  mga_accel_flush(&my_state);

  gettimeofday(&current_time,NULL);
	   
  actual_duration =
    (current_time.tv_sec - end_time.tv_sec + TEST_DURATION)
    + 1e-6 * (current_time.tv_usec - end_time.tv_usec);
	 
  printf("%i gouraud triangles in %f seconds, i.e. %f gtri/s\n",
	 no, actual_duration, ((float)no)/actual_duration);
	
  return 0;
}

