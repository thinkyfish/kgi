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
** Icosahedron test
*/

/* The icosahedron is centered in the unit sphere at (0,0,0)
 */
#define IX .525731112119133606
#define IZ .850650808352039932
static float ico_data[12][3] = {
  {-IX, 0.0, IZ}, {IX, 0.0, IZ}, {-IX, 0.0, -IZ}, {IX, 0.0, -IZ},
  {0.0, IZ, IX}, {0.0, IZ, -IX}, {0.0, -IZ, IX}, {0.0, -IZ, -IX},
  {IZ, IX, 0.0}, {-IZ, IX, 0.0}, {IZ, -IX, 0.0}, {-IZ, -IX, 0.0}
};
static kgi_u_t ico_indices[20][3] = {
  {1,4,0},{4,9,0},{4,5,9},{8,5,4},{1,8,4},
  {1,10,8},{10,3,8},{8,3,5},{3,2,5},{3,7,2},
  {3,10,7},{10,6,7},{6,11,7},{6,0,11},{6,1,0},
  {10,1,6},{11,0,9},{2,11,9},{5,2,9},{11,2,7}
};

static mga_vertex_color_t ico_color(float dist)
{
  mga_vertex_color_t c;
  c.alpha = 0;
  c.red = c.blue = c.green = ((int)(255.0 * (1.0 - dist)) & 0xFF);
  return c;
}

static void draw_icosahedron(mga_accel_state_t *state,
			     kgi_u_t xmin, kgi_u_t xmax,
			     kgi_u_t ymin, kgi_u_t ymax,
			     kgi_u_t texture)
{
  int i;
  for (i=0; i<20; i++)
    {
      mga_vertex_t v1,v2,v3;
      float x1 = ico_data[ico_indices[i][0]][0];
      float y1 = ico_data[ico_indices[i][0]][1];
      float z1 = ico_data[ico_indices[i][0]][2];
      float x2 = ico_data[ico_indices[i][1]][0];
      float y2 = ico_data[ico_indices[i][1]][1];
      float z2 = ico_data[ico_indices[i][1]][2];
      float x3 = ico_data[ico_indices[i][2]][0];
      float y3 = ico_data[ico_indices[i][2]][1];
      float z3 = ico_data[ico_indices[i][2]][2];
      /* We need to project: we do it onto plane a z=d with persp. */
      float d = 3.0;
      float xp1 = x1/(d-z1);
      float yp1 = y1/(d-z1);
      float xp2 = x2/(d-z2);
      float yp2 = y2/(d-z2);
      float xp3 = x3/(d-z3);
      float yp3 = y3/(d-z3);
      /* Now we build the vertices */
      float xl2 = (xmax - xmin) / 2;
      float yl2 = (ymax - ymin) / 2;
      float xc = xmin + xl2;
      float yc = ymin + yl2;
      float dzmax = 5.0;
      float zoom = 3.0; /* distance shows perspective, but also shrinks
			   the object.. so we zoom also */
      v1.x = (xp1 * xl2) * zoom + xc;
      v1.y = (yp1 * yl2) * zoom + yc;
      v1.z = (d - z1) / dzmax; /* Should be <1.0 ... */
      v2.x = (xp2 * xl2) * zoom + xc;
      v2.y = (yp2 * yl2) * zoom + yc;
      v2.z = (d - z2) / dzmax; /* Should be <1.0 ... */
      v3.x = (xp3 * xl2) * zoom + xc;
      v3.y = (yp3 * yl2) * zoom + yc;
      v3.z = (d - z3) / dzmax; /* Should be <1.0 ... */
      /* colors */
      v1.color = ico_color(v1.z);
      v2.color = ico_color(v2.z);
      v3.color = ico_color(v3.z);
      /* texture */
      v1.tu0 = 0.0;
      v1.tv0 = 0.0;
      v2.tu0 = 6.5;
      v2.tv0 = 0.0;
      v3.tu0 = 6.5;
      v3.tv0 = 6.5;
      /* persp. correction */
      v1.rhw = 1.0;
      v2.rhw = 1.0;
      v3.rhw = 1.0;
#if 1
      mga_accel_set_texture(state, texture);
      mga_accel_draw_tex_triangle(state, &v1,&v2,&v3);
#else
      printf("v1.x,y = (%f,%f) v1.z = %f v1.color=(%.2x,%.2x,%.2x,%.2x)\n"
	     "v2.x,y = (%f,%f) v2.z = %f v2.color=(%.2x,%.2x,%.2x,%.2x)\n"
	     "v3.x,y = (%f,%f) v3.z = %f v3.color=(%.2x,%.2x,%.2x,%.2x)\n\n",
	     v1.x,v1.y,v1.z,
	     v1.color.red,v1.color.green,v1.color.blue,v1.color.alpha,
	     v2.x,v2.y,v2.z,
	     v2.color.red,v2.color.green,v2.color.blue,v2.color.alpha,
	     v3.x,v3.y,v3.z,
	     v3.color.red,v3.color.green,v3.color.blue,v3.color.alpha);
#endif
    }
}

#define swap(x,y,type) { type tmp; tmp = (x); (x) = (y); (y) = tmp; }

/*
** DMA buffer size (do *not* change for the moment)
*/
#define BUFFER_SIZE_LN2 0
#define BUFFER_SIZE ((1 << BUFFER_SIZE_LN2) * 4 * 1024)
#define BUFFER_NUMBER_LN2 2 /* TODO: !!!!! */
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

  /* Do some testing of the raw buffer functions
   */
  {
    mga_dma_buffer_t b1, b2;
    char* bb1[8192];
    char* bb2[8192];
    int ii;

    mga_dma_buffer_init(&b1,(kgi_u32_t*)bb1,(kgi_u32_t*)(bb1+8192));
    mga_dma_buffer_reset(&b1,MGA_DMA_GENERAL_PURPOSE);
    for (ii = 0; ii < 17; ii++)
      {
	mga_dma_buffer_out_reg(&b1, ii, 0xFFF - (ii << 2));
      }
    mga_dma_buffer_printf_content(&b1);

    mga_dma_buffer_init(&b2,(kgi_u32_t*)bb2,(kgi_u32_t*)(bb2+8192));
    mga_dma_buffer_reset(&b2,MGA_DMA_VERTEX_TRIANGLE_LIST);
    for (ii = 0; ii < 4; ii++)
      {
	mga_vertex_t v1;
	kgi_u_t r = (rand() % 3);
	v1.x = (float)(rand() % 10000) * 0.1;
	v1.y = (float)(rand() % 7000)  * 0.1;
	v1.z = 0.01;
	v1.color = (r == 0) ? col1 : ((r==1) ? col2 : col3);
	v1.specular = spcol;
	v1.tu0 = (float)(rand() % 16);
	v1.tv0 = (float)(rand() % 16);
	mga_dma_buffer_out_vertex(&b2, &v1);
      }
    mga_dma_buffer_printf_content(&b2);
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
  ** Inits the textures
  ** (16x16 in 16bpp): 512 bytes per texture
  */
#define TEXTURE_NUMBER 12
  {
    int Tn,u,v;
    kgi_u16_t *ptr = (kgi_u16_t*)((kgi_u32_t)fb + my_state.free_space_offset);
    kgi_u16_t *p2 = (kgi_u16_t*)fb;

    p2 += 1024 - 16 - 1;
    printf("Free space FB offset: %.8x (%f MB)\n",
	   my_state.free_space_offset,
	   (float)my_state.free_space_offset / (1024.0 * 1024.0));

    for (Tn = 0; Tn < TEXTURE_NUMBER; Tn++)
      {
	kgi_u16_t c1 = rand() % 65535;
	kgi_u16_t c2 = rand() % 65535;
	printf(" texture %i at %.8x (c1=%.8x,c2=%.8x)\n", Tn, ptr, c1, c2);
	for (v = 0; v < 16; v++)
	  {
	    for (u = 0; u < 16; u++)
	      {
		kgi_u16_t cc;
		if (Tn % 4)
		  cc = (((u&8)==0)^((v&8)==0)) ? c1 : c2;
		else
		  cc = rand() % 65535;
		//(*ptr) = (((u * v) << (Tn)) | 0x1234);
		//(*ptr) = (v + 10) | (v << 5) | ((v + 12) << 11);
		//(*ptr) = (u ^ v) & 0xFFFF;
		(*ptr) = cc;
		ptr++;
		(*p2) = cc; p2++;;
	      }
	    p2 += 1024 - 16;
	  }
      }
  }

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
	int xmin = 375;
	int xmax = 950;
	int ymin = 550;
	int ymax = 750;

	int top,left,width,height;
	int xl,yl,xle,yle;
	mga_vertex_color_t color;
	mga_vertex_t vert[3];
	mga_vertex_t fixvert[3];

	left = xmin + (rand() % (xmax - xmin));
	width = (rand() % (xmax - left + 1)) + 1;
	top = ymin + (rand() % (ymax - ymin));
	height = (rand() % (ymax - top + 1)) + 1;
	xl = xmin + (rand() % (xmax - xmin));
	yl = ymin + (rand() % (ymax - ymin));
	xle = xmin + (rand() % (xmax - xmin));
	yle = ymin + (rand() % (ymax - ymin));
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
	mga_accel_draw_line(&my_state, xl, yl, xle, yle);

	for (i = 0; i < 3; i++)
	  {
	    kgi_u_t r = (rand() % 3);
	    vert[i].rhw = 1.0;
	    vert[i].tu0 = 0.0;
	    vert[i].tv0 = 0.0;
	    vert[i].x = (float)(rand() % 3000) * 0.1;
	    vert[i].y = (float)(rand() % 5500)  * 0.1;
	    vert[i].z = (float)(rand() % 99) * 0.01; // 0.0; // 0.01;
	    //vert[i].color = (r == 0) ? col1 : ((r==1) ? col2 : col3);
	    vert[i].color = (i == 0) ? col1 : ((i==1) ? col2 : col3);
	    vert[i].specular = black;
	    vert[i].specular = spcol; /* strange effect */
	  }
	mga_accel_draw_triangle(&my_state, &(vert[0]), &(vert[1]), &(vert[2]));

	for (i = 0; i < 3; i++)
	  {
	    kgi_u_t r = (rand() % 3);
	    vert[i].rhw = 0.9;
	    vert[i].tu0 = ((rand() % 500) * 0.01); // 0.5;
	    vert[i].tv0 = ((rand() % 500) * 0.01); // 0.5;
	    vert[i].x = (float)(rand() % 6500) * 0.1 + 300.0;
	    vert[i].y = (float)(rand() % 6000) * 0.1;
	    vert[i].z = (float)(rand() % 90) * 0.01; // 0.0; // 0.01;
	    //vert[i].color = (r == 0) ? col1 : ((r==1) ? col2 : col3);
	    vert[i].color = (i == 0) ? col1 : ((i==1) ? col2 : col3);
	    vert[i].specular = black;
	  }
#define TEXTURE_OF_TRIANGLES ((rand() % TEXTURE_NUMBER) & 0x1F)
	mga_accel_set_texture(&my_state, TEXTURE_OF_TRIANGLES);
	mga_accel_draw_tex_triangle(&my_state,
				    &(vert[0]), &(vert[1]), &(vert[2]));

#if 1

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

	mga_accel_set_texture(&my_state, TEXTURE_OF_TRIANGLES);
	mga_accel_draw_tex_triangle(&my_state,
				    &(fixvert[0]), &(fixvert[1]), &(fixvert[2]));
#endif

#if 1
	draw_icosahedron(&my_state, 10, 330, 575, 760,
			 TEXTURE_OF_TRIANGLES);
#endif

#if 0
	mga_accel_flush(&my_state);
	exit(1);
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
	 
  printf("%i XXXX in %f seconds, i.e. %f XXX/s\n",
	 no, actual_duration, ((float)no)/actual_duration);
	
  return 0;
}

