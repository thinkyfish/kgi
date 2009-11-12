/*-
 * Copyright (c) 1995-1997 Andreas Beck
 * Copyright (c) 1995-2000 Steffen Seeger
 * Copyright (c) 2002-2004 Nicholas Souchu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and permit to persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,EXPRESSED OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_VM
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgi/graphic.h>

/*
 * command resource
 */
int 
graph_command(graph_file_t *file, unsigned int cmd, void *data,
		struct thread *td)
{
	if (cmd == KGIC_MAPPER_IDENTIFY) {
			/*
			 * For now only force client identification and
			 * identify myself (no compatibility checks).
			 */
			kgic_mapper_identify_result_t *out = data;

			if (file->flags & GRAPH_FF_CLIENT_IDENTIFIED)
				return (KGI_EPROTO);

			file->flags |= GRAPH_FF_CLIENT_IDENTIFIED;

			memset(out->mapper, 0, sizeof(out->mapper));
			strncpy(out->mapper, GRAPH_NAME, sizeof(out->mapper));
			out->mapper_version.major = 0;
			out->mapper_version.minor = 9;
			out->mapper_version.patch = 0;
			out->mapper_version.extra = 0;
			out->resources = __KGI_MAX_NR_RESOURCES;
			/* 
			 * XXX 
			 * #warning tell client if it is session leader or not.
			 */

			return (KGI_EOK);
	}

	/* all commands below require identification */
	if (!(file->flags & GRAPH_FF_CLIENT_IDENTIFIED)) {
		KGI_ERROR("cmd = %.8x, but client has not yet identified", cmd);
		return (KGI_EPROTO);
	}
	switch (cmd) {
	case KGIC_MAPPER_SET_IMAGES: {
		/* 
		 * Only the session leader may set the number of
		 * images as long as device isn't registered.
		 */
		kgic_mapper_set_images_request_t *in = data;
		kgi_size_t size;
		
		if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
			KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
			KGI_ERROR(
				(file->flags & GRAPH_FF_SESSION_LEADER)
				? "mode already checked"
				: "client is not session leader");
			return (KGI_EPROTO);
		}
		/* XXX #warning what about resources still mapped?! */
		if (file->device->kgi.mode) {
			kgi_kfree(file->device->kgi.mode->dev_mode);
			file->device->kgi.mode->dev_mode = NULL;
			kgi_kfree(file->device->kgi.mode);
			file->device->kgi.mode = NULL;
		}

		if ((in->images < 1) ||
			(GRAPH_MAX_NR_IMAGES < in->images)) {
			KGI_ERROR("no mem");
			return (KGI_ENOMEM);
		}
		size = sizeof(kgi_mode_t) + 
			sizeof(kgi_image_mode_t) * (in->images - 1);
		file->device->kgi.mode = kgi_kmalloc(size);
		if (file->device->kgi.mode == NULL) {
			KGI_ERROR("no mem");
			return (KGI_ENOMEM);
		}
		memset(file->device->kgi.mode, 0, size);
		file->device->kgi.mode->revision = KGI_MODE_REVISION;
		file->device->kgi.mode->images = in->images;
		return (KGI_EOK);
	}
	case KGIC_MAPPER_GET_IMAGES: {
		kgic_mapper_get_images_result_t *out = data;

		out->images = file->device->kgi.mode 
			? file->device->kgi.mode->images : 0;
		return (KGI_EOK);
	}
	case KGIC_MAPPER_SET_IMAGE_MODE: {
		kgic_mapper_set_image_mode_request_t *in = data;

		if ((!(file->flags & GRAPH_FF_SESSION_LEADER)) ||
			KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
			KGI_ERROR(
				(file->flags & GRAPH_FF_SESSION_LEADER)
				? "mode already checked"
				: "client is not session leader");
			return (KGI_EPROTO);
		}

		if (file->device->kgi.mode->images <= in->image) 
			return (KGI_EINVAL);

		file->device->kgi.mode->img[in->image] = in->mode;

		return (KGI_EOK);
	}
	case KGIC_MAPPER_GET_IMAGE_MODE: {
		kgic_mapper_get_image_mode_request_t local = 
			*(kgic_mapper_get_image_mode_request_t *)data;
		kgic_mapper_get_image_mode_request_t *in = &local;
		kgic_mapper_get_image_mode_result_t *out = data;

		if (file->device->kgi.mode == NULL) {
			KGI_ERROR("number of images not yet set");
			return (KGI_EPROTO);
		}
		if (file->device->kgi.mode->images <= in->image) 
			return (KGI_EINVAL);

		out->mode = file->device->kgi.mode->img[in->image];
		return (KGI_EOK);
	}
	case KGIC_MAPPER_MODE_CHECK: {
#if defined(KGI_DBG_LEVEL) && (KGI_DBG_LEVEL > 0)
		kgi_u_t i;
#endif
		kgi_error_t err;
		if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
			(file->device->kgi.mode == NULL)) {
			KGI_ERROR(
				(file->flags & GRAPH_FF_SESSION_LEADER)
				? "number of images not yet set"
				: "client is not session leader");
			return (KGI_EPROTO);
		}
		KGI_ASSERT(!KGI_VALID_DEVICE_ID(file->device->kgi.id));
		KGI_ASSERT(!KGI_VALID_DISPLAY_ID(file->device->kgi.dpy_id));

		file->device->kgi.MapDevice   = graph_device_map;
		file->device->kgi.UnmapDevice = graph_device_unmap;
		file->device->kgi.HandleEvent = NULL;

		err = kgi_register_device(&(file->device->kgi), 
		file->device_id);
		if (KGI_EOK != err) {
			KGI_ERROR("Failed to register device (%i)", err);
			return (err);
		}
#if defined(KGI_DBG_LEVEL) && (KGI_DBG_LEVEL > 0)
		i = 0;
		while (i < __KGI_MAX_NR_RESOURCES) {
			const kgi_resource_t *r =
				file->device->kgi.mode->resource[i];

			if (r == NULL) 
				break;

			KGI_DEBUG(1, "resource %i (%s) has type %.8x",
				i, r->name, r->type);
			i++;
		}
#endif
		return (KGI_EOK);
	}
	case KGIC_MAPPER_MODE_SET: {
		kgi_device_t *prev;
		if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
			!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
				KGI_ERROR(
					(file->flags & GRAPH_FF_SESSION_LEADER) ?
					"mode not yet checke.d" :
					"client is not session leader.");
				return (KGI_EPROTO);
		}
		prev = kgi_current_focus(file->device->kgi.dpy_id);
		if (prev) {
			KGI_DEBUG(1, "unmapping previous device.");
			switch (kgi_unmap_device(prev->id)) {
			case KGI_EOK:
				file->previous = prev->id;
				break;
			default:
				KGI_ERROR("can't unmap current focus.");
				return (KGI_EBUSY);
			}
		}
		kgi_map_device(file->device->kgi.id);
		KGI_DEBUG(1, "mapping new device.");
		return (KGI_EOK);
	}
	case KGIC_MAPPER_MODE_UNSET: {
		if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
			!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
				KGI_ERROR(
					(file->flags & GRAPH_FF_SESSION_LEADER) ?
					"mode not yet checked" :
					"client is not session leader");
				return (KGI_EPROTO);
		}
		kgi_unmap_device(file->device->kgi.id);
		return (KGI_EOK);
	}
	case KGIC_MAPPER_MODE_DONE: {
		if (!(file->flags & GRAPH_FF_SESSION_LEADER) ||
			!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
				KGI_ERROR(
					(file->flags & GRAPH_FF_SESSION_LEADER) ?
					"mode not yet checked" :
					"client is not session leader");
				return (KGI_EPROTO);
		}
		if (file->device->kgi.flags & KGI_DF_FOCUSED) {
			kgi_unmap_device(file->device->kgi.id);
			if (KGI_VALID_DEVICE_ID(file->previous))
				kgi_map_device(file->previous);
			}
			kgi_unregister_device(&(file->device->kgi));
			return (KGI_EOK);
		}
	case KGIC_MAPPER_RESOURCE_INFO: {
		kgic_mapper_resource_info_request_t local = 
			*(kgic_mapper_resource_info_request_t *)data;
		kgic_mapper_resource_info_request_t *in = &local;
		kgic_mapper_resource_info_result_t *out = data;
		const union {
			kgi_resource_t		common;
			kgi_mmio_region_t	mmio;
			kgi_accel_t		accel;
			kgi_shmem_t		shmem;
		} *r;

		if (!KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
			KGI_ERROR("mode not yet checked");
			return (KGI_EAGAIN);
		}
		if (((in->image == KGIC_MAPPER_NON_IMAGE_RESOURCE)?
			__KGI_MAX_NR_IMAGE_RESOURCES:
			__KGI_MAX_NR_RESOURCES) <= in->resource) {
			KGI_ERROR("invalid resource ID %d", in->resource);
			return (KGI_EINVAL);
		}
		if ((in->image < 0) ||
			(file->device->kgi.mode->images <= in->image)) {
			if (in->image!=KGIC_MAPPER_NON_IMAGE_RESOURCE) {
				KGI_ERROR("invalid image %d (%i)", 
					in->image, in->image);
				return (KGI_EINVAL);
			}
		}

		r = (in->image == KGIC_MAPPER_NON_IMAGE_RESOURCE) 
			? (void*)file->device->kgi.mode->resource[in->resource]
			: (void*)file->device->kgi.mode->img[in->image]
			  .resource[in->resource];

		if (r) {
			memset(out, 0, sizeof(*out));

			strncpy(out->name, r->common.name, sizeof(out->name));
			out->name[sizeof(out->name) - 1] = 0;
			out->resource = in->resource;
			out->image = in->image;
			out->type = r->common.type;
			out->protection = r->common.prot;
			switch (r->common.type & KGI_RT_MASK) {
			case KGI_RT_MMIO:
				out->info.mmio.access = r->mmio.access;
				out->info.mmio.align  = r->mmio.align;
				out->info.mmio.size   = r->mmio.size;
				out->info.mmio.window = r->mmio.win.size;
				break;
			default:
				KGI_ERROR(
					"unknown resource type %.8x",
					r->common.type & KGI_RT_MASK);
			}
			return (KGI_EOK);
		} else {
			KGI_ERROR("no such resource %i", in->resource);
			return (KGI_ENXIO);
		}
	}
	case KGIC_MAPPER_MMAP_SETUP: {
		kgic_mapper_mmap_setup_request_t *in = data;

		KGI_DEBUG(1, "mmap setup.");

		if (! KGI_VALID_DEVICE_ID(file->device->kgi.id)) {
			KGI_ERROR("mode not yet checked.");
			return (KGI_EAGAIN);
		}
		if (file->mmap_setup.resource &&
			(file->mmap_setup.pid != td->td_proc->p_pid ||
			 file->mmap_setup.gid != td->td_proc->p_pgrp->pg_id)) {
				KGI_ERROR("only initiator can change mmap"
						" setup.");
			return (KGI_EPERM);
		}

		file->mmap_setup.resource = NULL;
		file->mmap_setup.pid = 0;
		file->mmap_setup.gid = 0;

		if (in->image == -1) {
			if (__KGI_MAX_NR_RESOURCES <= in->resource) {
				KGI_ERROR("invalid resource ID.");
				return (KGI_EINVAL);
			}
		if (!file->device->kgi.mode->resource[in->resource]) {
			KGI_ERROR("no such resource %i", in->resource);
			return (KGI_ENXIO);
		}

		file->mmap_setup.resource = 
			file->device->kgi.mode->resource[in->resource];
		} else {
			if (in->image > file->device->kgi.mode->images) {
				KGI_ERROR("no such image %i", in->image);
				return (KGI_ENXIO);
			}
			if (__KGI_MAX_NR_IMAGE_RESOURCES <= in->resource) {
				KGI_ERROR("invalid resource ID.");
				return (KGI_EINVAL);
			}
			if (!file->device->kgi.mode->
				img[in->image].resource[in->resource]) { 
					KGI_ERROR("no such resource %i",
					 in->resource);
				return (KGI_ENXIO);
			}

			file->mmap_setup.resource = 
				file->device->kgi.mode->resource[in->resource];
		}

		file->mmap_setup.request = *in;
		file->mmap_setup.pid = td->td_proc->p_pid;
		file->mmap_setup.gid = td->td_proc->p_pgrp->pg_id;

		KGI_DEBUG(1, "setup resource %d (at %p)", in->resource,
			  file->mmap_setup.resource);

		return (KGI_EOK);
	}
	default:
		KGI_DEBUG(1, "command %.4x not (yet) implemented.", cmd);
		return (KGI_ENXIO);
	}
}
