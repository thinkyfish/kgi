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
#define TEST_DURATION 30

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


static int dma_size = (3 * (4 * 5)) + MGAG_ACCEL_TAG_LENGTH;
static kgi_u32_t dma_cmdlist[] =
{
  MGAG_ACCEL_TAG_DRAWING_ENGINE,
#if MYSTIQUE
  MGA_DMA4(PITCH,YDSTORG,MACCESS,CXBNDRY),
#else
  MGA_DMA4(PITCH,DSTORG,MACCESS,CXBNDRY),
#endif
  0x00000400, /* PITCH: ylin=0, iy=1024 */
  0x00000000, /* DSTORG or YDSTORG */
  0x40000001, /* MACCESS */
  0x0FFF0000, /* CXBNDRY */
  MGA_DMA4(YTOP,YBOT,PLNWT,ZORG),
  0x00000000, /* YTOP */
  0x00FFFFFF, /* YBOT */
  0xFFFFFFFF, /* PLNWT */
  0x00080000, /* ZORG (overlaps with fb! not used normally)*/
#if 0 /* Line */
  0x00091110,
  0x000A000A, /* XYSTRT */
  0x02000300, /* XYEND */
  0xE2E2E2E2, /* FCOL */
  0x840C4813, /* DWGCTL: AUTOLINE_CLOSE */
#else /* Rectangle fill */
  MGA_DMA4(FXBNDRY,YDSTLEN,FCOL,DWGCTL|ACCEL_GO),
  0x03800040, /* FXBNDRY */
  0x01800070, /* YDSTLEN */
  0x21212121, /* FCOL */
#if 0
  0xC40C7814, /* DWGCTL (via 0x1D00) */
#else
  0xC00C7814, /* DWGCTL (via 0x1D00) */  
#endif
#endif
};

typedef struct _color_s {
  kgi_u8_t blue, green, red, alpha;
} color_t;

typedef struct _vertex_s {
  float x; /* screen coordinates */
  float y;
  float z; /* Z buffer depth, in 0.0 - 1.0 */
  float rhw; /* 1/w (reciprocal of homogeneous w) */
  color_t color; /* vertex color */
  color_t specular; /* specular component. Alpha is fog factor */
  float tu0; /* texture coordinate stage 0 */
  float tv0;
} vertex_t;

typedef struct _vertex2_s {
  float x; /* screen coordinates */
  float y;
  float z; /* 0.0 - 1.0 */
  float rhw; /* 1/w (reciprocal of homogeneous w) */
  color_t color; /* vertex color */
  color_t specular; /* specular component. Alpha is fog factor */
  float tu0; /* texture coordinate stage 0 */
  float tv0;
  float tu1; /* texture coordinate stage 1 */
  float tv1;
} vertex2_t;

#define swap(x,y,type) { type tmp; tmp = (x); (x) = (y); (y) = tmp; }

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
	vertex_t avertex;
	kgi_u32_t warp_tag;

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
	  int h, hend = 70 + 20 * (random() & 15);

	  for (y = 0; y < 768; y++) 
	    for (x = 0; x < 1024; x++) {
	      
	      fb[y*mode.virt.x + x] =
		// 0x7000 | (y * 16) | ((x + i) & 15);
		0 + x & 7;
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
	/* We have two buffers */
#if 0
	ping = accel;
	pong = accel + BUFFER_SIZE;
#endif

	//	{ int cnt = 1000000000; while (cnt--) { int i; i++; }; }

	wping = accel;
	wpong = accel + BUFFER_SIZE;

	// { int cnt = 100000; while (cnt--) { int i; i++; }; }

	gettimeofday(&end_time,NULL);
	end_time.tv_sec += TEST_DURATION;

	do
	{
#if 1
	  dma_cmdlist[12] = 0x03800040;
	  dma_cmdlist[13] = 0x01800080;
	  i = 3;
	  dma_cmdlist[14] = i | (i<<8) | (i<<16) | (i<<24);
	  memcpy(wping, dma_cmdlist, dma_size);
	  (*(wpong + dma_size)) = 0x15; /* Tells size */
	  swap(wping,wpong,kgi_u8_t*);
#endif

	  /*
	  ** Sends vertices
	  */
	  /* No texture */
	  avertex.rhw = 1.0;
	  avertex.tu0 = 0.0;
	  avertex.tv0 = 0.0;
	  /* buffer tag */
	  warp_tag = MGAG_ACCEL_TAG_WARP_TGZ;
	  {
	    color_t col1 = { 0, 255, 0, 12 }; /* Green, semi opaque */
	    color_t col2 = { 255,0,0,12 }; /* Blue, semi opaque */
	    color_t col3 = { 0,0,255,12 }; /* Red, semi opaque */
	    color_t spcol = { 128,128,128,128 }; /* Gray, fog=128 */
	    
	    ptr = wping;
	    memcpy(ptr, &warp_tag, MGAG_ACCEL_TAG_LENGTH);
	    ptr += MGAG_ACCEL_TAG_LENGTH;
	    while ((BUFFER_SIZE - 1 - (ptr - wping)) > (3*sizeof(avertex)))
	      {
		int j;
		for (j = 0; j < 3; j++)
		  {
		    kgi_u_t r = (rand() % 3);
		    /* Triangle 1 */
		    avertex.x = (float)(rand() % 10000) * 0.1;
		    avertex.y = (float)(rand() % 7000)  * 0.1;
		    avertex.z = 0.0; // 0.01;
		    avertex.color = (r == 0) ? col1 : ((r==1) ? col2 : col3);
		    avertex.specular = spcol;
		    memcpy(ptr,&avertex,sizeof(avertex));
		    ptr += sizeof(avertex);
		    no++;
		  }
	      }	  
	  }
	  /* printf("wpong + %.4x\n", (ptr - wping)); */
	  /* Starts execution */
	  (*(wpong + (ptr - wping))) = 0x0; /* Tells size */
	  swap(wping,wpong,kgi_u8_t*);

	  gettimeofday(&current_time,NULL);
	}
	while (timercmp(&current_time,&end_time,<));

	actual_duration =
	  (current_time.tv_sec - end_time.tv_sec + TEST_DURATION)
	  + 1e-6 * (current_time.tv_usec - end_time.tv_usec);
	
	printf("%i triangles in %f seconds, i.e. %f tri/s\n",
	       no, actual_duration, ((float)no)/actual_duration);
	
	return 0;
}
