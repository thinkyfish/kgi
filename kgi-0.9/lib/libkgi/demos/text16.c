#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <asm/semaphore.h>

//#include <kgi/kgi.h>
//#include <kgi/graphic.h>

#include <libkgi/libkgi.h>

int main(int argc, char *argv[])
{
	kgi_context_t ctx;
	kgi_version_t	testKGI_version = { 0, 0, 1, 0 };
	kgi_image_mode_t mode;
	kgi_u_t	i, x, y;
	kgi_u16_t *fb;

	kgiInit(&ctx, "testKGI", &testKGI_version);

	kgiSetImages(&ctx, 1);
	
	memset(&mode, 0, sizeof(mode));
	
	mode.flags |= KGI_IF_TEXT16;
	
	mode.size.x = mode.virt.x = 80;
	mode.size.y = mode.virt.y = 30;
	
	fprintf(stderr, "mode.out = %p\n", mode.out);
	
	fprintf(stderr, "mode.out = %p\n", mode.out);
		
	kgiSetImageMode(&ctx, 0, &mode);
	
	fprintf(stderr, "mode.out = %p\n", mode.out);
	kgiCheckMode(&ctx);
	
	fprintf(stderr, "mode.out = %p\n", mode.out);
	kgiGetImageMode(&ctx, 0, &mode);
	
	fprintf(stderr, "mode.out = %p\n", mode.out);
	kgiPrintImageMode(&mode);
	
	i = 0;
	while (kgiPrintResourceInfo(&ctx, i) == KGI_EOK) 
	{
		fprintf(stderr, "i = %d\n", i);
		i++;
	}

	kgiSetMode(&ctx);

	fb = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED,
		  ctx.mapper.fd, 
		  GRAPH_MMAP_TYPE_MMIO | (0 << GRAPH_MMAP_RESOURCE_SHIFT));
	
	fprintf(stderr, "fb = %.8x\n", fb);

	if (fb == MAP_FAILED)
	{
		fprintf(stderr, "Failed to mmap() the framebuffer\n");
		perror("mmap() error: ");
		exit(1);
	}
	
	
	memset(fb, 0xff, 0x100);

#if 1
	for (i = 0; i < 1000000000; i++) 
	{
		int h, hend = 700 + 1000 * (random() & 15);

		for (y = 0; y < 16; y++) 
		for (x = 0; x < 16; x++) 
		{
			fb[y * 80 + x] = 0x7000 | (y * 16) | ((x + i) & 15);
			
			for (h = 0; h < hend; h++);
		}
	}
#endif
	
	kgiUnsetMode(&ctx);
	
	return 0;
}

