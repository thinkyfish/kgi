#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

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

	kgiGetImageMode(&ctx, 0, &mode);
	kgiPrintImageMode(&mode);

	i = 0;
	while (kgiPrintResourceInfo(&ctx, i) == KGI_EOK) {

		i++;
	}

	fb = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED,
		ctx.mapper.fd, 
		GRAPH_MMAP_TYPE_MMIO | (0 << GRAPH_MMAP_RESOURCE_SHIFT));

	for (i = 0; i < 100000000000; i++) {

		for (y = 0; y < 16; y++) 
		for (x = 0; x < 16; x++) {

			int h;

			fb[y*80 + x + 40] = 0x0600 | (((y + i)&15) * 16) | x;
			for (h = 0; h < 10000; h++);
		}
	}

	return 0;
}

