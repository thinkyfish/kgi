/*
**	This is a little, very little test program to test the accelerator
**	mapping code. Only useful with the dpy-i386 blubber accelerator.
*/
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
	kgi_u8_t *accel, *ping, *pong;

	kgiInit(&ctx, "testKGI", &testKGI_version);

	kgiSetImages(&ctx, 1);

	memset(&mode, 0, sizeof(mode));
	mode.flags |= KGI_IF_TEXT16;
	kgiSetImageMode(&ctx, 0, &mode);

	kgiCheckMode(&ctx);

	kgiGetImageMode(&ctx, 0, &mode);
	kgiPrintImageMode(&mode);

	i = 0;
	while (kgiPrintResourceInfo(&ctx, i) == KGI_EOK) {

		i++;
	}

	kgiSetMode(&ctx);

	if (accel = mmap(NULL, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED,
		ctx.mapper.fd, 
		GRAPH_MMAP_TYPE_ACCEL | 0x00101100)) 

	ping = accel;
	pong = accel + 4096;

	strcpy(ping, "first ping-buffer");
	strcpy(pong, "first pong-buffer");
	strcpy(ping, "second ping-buffer");
	strcpy(pong, "second pong-buffer");

	return 0;
}

