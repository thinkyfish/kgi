#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <kii/kii.h>

struct kii_context_s
{
	struct {
		int	fd;
	} mapper;

	struct {

		kii_u_t  size, curr;
		kii_u8_t buffer[1024];

	} evbuf;
	kiic_mapper_get_keymap_info_result_t keymap_info;
};

kii_error_t kiiInit(kii_context_t **ctx)
{
	union {
		kiic_mapper_identify_request_t	request;
		kiic_mapper_identify_result_t	result;
	} identify;

	if (NULL == ctx) {

		return -KII_ERRNO(LIB, INVAL);
	}
	*ctx = calloc(1, sizeof(**ctx));
	if (NULL == *ctx) {

		return -KII_ERRNO(LIB, NOMEM);
	}

	(*ctx)->mapper.fd = open("/dev/event", O_RDWR | O_NONBLOCK);
	if ((*ctx)->mapper.fd < 0) {

		perror("failed to open /dev/event: ");
		free(*ctx); *ctx = NULL;
		return -KII_ERRNO(LIB, INVAL);
	}

	memset(&identify, 0, sizeof(identify));
	strncpy(identify.request.client, "libkii",
		sizeof(identify.request.client));
	identify.request.client[sizeof(identify.request.client) - 1] = 0;
	identify.request.client_version.major = 0;
	identify.request.client_version.minor = 0;
	identify.request.client_version.patch = 0;
	identify.request.client_version.extra = 0;

	if (ioctl((*ctx)->mapper.fd, KIIC_MAPPER_IDENTIFY, &identify) < 0) {

		perror("failed to identify to mapper: ");
		free(*ctx); *ctx = NULL;
		return errno;
	}

	fprintf(stderr, "identified to mapper %s-%i.%i.%i-%i\n",
		identify.result.mapper,
		identify.result.mapper_version.major,
		identify.result.mapper_version.minor,
		identify.result.mapper_version.patch,
		identify.result.mapper_version.extra);

	
	memset(&(*ctx)->keymap_info, 0, sizeof((*ctx)->keymap_info));
	if (ioctl((*ctx)->mapper.fd, KIIC_MAPPER_GET_KEYMAP_INFO,
		&(*ctx)->keymap_info) < 0) {

		perror("failed to get keymap info: ");
		free(*ctx); *ctx = NULL;
		return errno;
	}

	fprintf(stderr, "keymap info: fn_buf_size %i, fn_str_size %i, "
		"keymin %i, keymax %i, keymap_size %i, combine_size %i",
		(*ctx)->keymap_info.fn_buf_size,
		(*ctx)->keymap_info.fn_str_size,
		(*ctx)->keymap_info.keymin,
		(*ctx)->keymap_info.keymax,
		(*ctx)->keymap_info.keymap_size,
		(*ctx)->keymap_info.combine_size);

	return KII_EOK;
}

kii_error_t kiiMapDevice(kii_context_t *ctx)
{
	return (ioctl(ctx->mapper.fd, KIIC_MAPPER_MAP_DEVICE, NULL) == 0)
		? KII_EOK : errno;
}

inline kii_u_t kiiEventAvailable(kii_context_t *ctx)
{
	kii_event_t *event;
	ssize_t count;

	event = (kii_event_t *) (ctx->evbuf.buffer + ctx->evbuf.curr);

	if (ctx->evbuf.size) {

		/*	if the next event is completely in buffer, we have a
		**	event available.
		*/
		if (event->size <= ctx->evbuf.size - ctx->evbuf.curr) {

			return 1;
		}

		/*	if not, move the event fraction to the start of 
		**	the buffer
		*/
		memmove(ctx->evbuf.buffer, 
			ctx->evbuf.buffer + ctx->evbuf.curr,
			ctx->evbuf.size - ctx->evbuf.curr);
		ctx->evbuf.size -= ctx->evbuf.curr;
		ctx->evbuf.curr = 0;
		event = (kii_event_t *) ctx->evbuf.buffer;
	}

	/*	attempt to read new event data
	*/
	count = read(ctx->mapper.fd,
			ctx->evbuf.buffer + ctx->evbuf.size,
			sizeof(ctx->evbuf.buffer) - ctx->evbuf.size);

	if (0 < count) {

		ctx->evbuf.size += count;
	}

	return ctx->evbuf.size &&
		(event->size <= ctx->evbuf.size - ctx->evbuf.curr);
}

int kiiEventDeviceFD(kii_context_t *ctx)
{
	return ctx->mapper.fd;
}

const kii_event_t *kiiNextEvent(kii_context_t *ctx)
{
	kii_event_t *event;

	event = (kii_event_t *) (ctx->evbuf.buffer + ctx->evbuf.curr);

	if (ctx->evbuf.size &&
		(event->size <= ctx->evbuf.size - ctx->evbuf.curr)) {

		ctx->evbuf.curr += event->size;
		if (ctx->evbuf.curr == ctx->evbuf.size) {

			ctx->evbuf.size = ctx->evbuf.curr = 0;
		}

		return event;

	}

	return NULL;
}

void kiiPrintEvent(kii_context_t *kii, FILE *f, const kii_event_t *e)
{
	fprintf(f, "event: size %i, focus %i, device %i, time %i",
		e->any.size, e->any.focus, e->any.device, e->any.time);

	switch (e->any.type) {

	case KII_EV_COMMAND:
		fprintf(f, " COMMAND\n");
		break;

	case KII_EV_BROADCAST:
		fprintf(f, " BROADCAST\n");
		break;

	case KII_EV_DEVICE_INFO:
		fprintf(f, " DEVICE_INFO\n");
		break;

	case KII_EV_RAW_DATA:
		fprintf(f, " RAW_DATA\n");
		break;

	case KII_EV_KEY_PRESS:
		fprintf(f, " KEY_PRESS\n");
		break;

	case KII_EV_KEY_RELEASE:
		fprintf(f, " KEY_RELEASE\n");
		break;

	case KII_EV_KEY_REPEAT:
		fprintf(f, " KEY_REPEAT\n");
		break;

	case KII_EV_KEY_STATE:
		fprintf(f, " KEY_STATE\n");
		break;

	case KII_EV_PTR_RELATIVE:
		fprintf(f, " PTR_RELATIVE\n");
		break;

	case KII_EV_PTR_ABSOLUTE:
		fprintf(f, " PTR_ABSOLUTE\n");
		break;

	case KII_EV_PTR_BUTTON_PRESS:
		fprintf(f, " PTR_BUTTON_PRESS\n");
		break;

	case KII_EV_PTR_BUTTON_RELEASE:
		fprintf(f, " PTR_BUTTON_RELEASE\n");
		break;

	case KII_EV_PTR_STATE:
		fprintf(f, " PTR_STATE\n");
		break;

	case KII_EV_VAL_RELATIVE:
		fprintf(f, " PTR_VAL_RELATIVE\n");
		break;

	case KII_EV_VAL_ABSOLUTE:
		fprintf(f, " PTR_VAL_ABSOLUTE\n");
		break;

	case KII_EV_VAL_STATE:
		fprintf(f, " VAL_STATE\n");
		break;

	case KII_EV_NOTHING:
		fprintf(f, " NOTHING\n");
		break;
	}
	
}








kii_u_t kiiLegalModifier(kii_context_t *kii, kii_u_t device, kii_u32_t key)
{
	return 1;
}

void kiiGetu(kii_context_t *kii, kii_enum_t var, kii_u_t *val)
{
	switch (var) {

	case KII_KBD_MIN_KEYCODE:
		*val = kii->keymap_info.keymin;
		return;

	case KII_KBD_MAX_KEYCODE:
		*val = kii->keymap_info.keymax;
		return;

	case KII_KBD_MAX_MAPSIZE:
		*val = kii->keymap_info.keymap_size;
		return;

	default:
		*val = 0;
		return;
	}
}




kii_error_t kiiGetKeymap(kii_context_t *kii, kii_unicode_t *map,
	kii_u_t keymap, kii_u_t keymin, kii_u_t keymax)
{
	union {
		kiic_mapper_get_keymap_request_t	request;
		kiic_mapper_get_keymap_result_t		result;
	} get_keymap;
	kii_u_t key;

	if ((NULL == map) || (keymax < keymin)) {

		return -EINVAL;
	}

	get_keymap.request.keymap = keymap;
	get_keymap.request.keymin = keymin;
	get_keymap.request.keymax = keymax;

	if (ioctl(kii->mapper.fd, KIIC_MAPPER_GET_KEYMAP, &get_keymap) != 0) {

		return errno;
	}

	for (key = keymin; key <= keymax; key++) {

		if ((key < get_keymap.result.keymin) ||
			(key > get_keymap.result.keymax)) {

			map[key - keymin] = K_VOID;
			continue;
		}
		map[key - keymin] = 
			get_keymap.result.map[key - get_keymap.result.keymin];
	}
	return KII_EOK;
}
