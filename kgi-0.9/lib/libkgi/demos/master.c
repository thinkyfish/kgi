#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <asm/semaphore.h>

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
	
//	mode.fam |= KGI_AM_COLORS;
	mode.fam |= KGI_AM_COLOR_INDEX;
	mode.bpfa[0] = 8;
	mode.size.x = mode.virt.x = 320;
	mode.size.y = mode.virt.y = 200;
	
	kgiSetImageMode(&ctx, 0, &mode);
	kgiCheckMode(&ctx);
	kgiGetImageMode(&ctx, 0, &mode);
	kgiPrintImageMode(&mode);

	i = 0;
	while (kgiPrintResourceInfo(&ctx, i) == KGI_EOK) 
	{
		i++;
	}

	kgiSetMode(&ctx);

	fb = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED,
		  ctx.mapper.fd, 
		  GRAPH_MMAP_TYPE_MMIO | (0 << GRAPH_MMAP_RESOURCE_SHIFT));
	
	if (fb == MAP_FAILED)
	{
		fprintf(stderr, "Failed to mmap() the framebuffer\n");
		perror("mmap() error: ");
		exit(1);
	}
	
#if 0

	memset(fb, 0xff, 0x10000);

	for (i = 0; i < 1000000000; i++) 
	{
		int h, hend = 700 + 1000 * (random() & 15);

		for (y = 0; y < 16; y++) 
		for (x = 0; x < 16; x++) 
		{
			fb[y * mode.virt.x + x] = 0x7000 | (y * 16) | ((x + i) & 15);
			
			for (h = 0; h < hend; h++);
		}
	}
	
#endif

	return 0;
}
