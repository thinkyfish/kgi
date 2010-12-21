/*-
 * Copyright (c) 1998-2000 Steffen Seeger
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

/*
 * /dev/graphic resource mapper definition.
 */
#ifndef _KGI_GRAPHIC_H_
#define	_KGI_GRAPHIC_H_

#define	GRAPH_MAJOR	60
#define	GRAPH_NAME	"graphic"

typedef struct graph_file_s	graph_file_t;
typedef struct graph_device_s	graph_device_t;
typedef struct graph_mapping_s	graph_mapping_t;

#define	__GRAPH_RESOURCE_MAPPING					  \
	graph_file_t		*file;	/* the file this belongs to	*/\
	graph_device_t		*device;/* the device this belongs to	*/\
	graph_mapping_t		*next;	/* mappings of same file	*/\
	graph_mapping_t		*other;	/* mappings of same resource	*/\
					/* belonging to the same device */\
	struct vm_area_struct   *vma;   /* the vm area of the map       */\
	kgi_resource_t		*resource; /* the resource mapped	*/

struct graph_mapping_s {
	__GRAPH_RESOURCE_MAPPING
};

/*
 * A device represents the actual device.
 */
struct graph_device_s {
	kgi_device_t	kgi;
	graph_mapping_t	*mappings[__KGI_MAX_NR_RESOURCES];
	kgi_mutex_t	cmd_mtx;
};

/*
 * A file is a particular virtual view of a device.
 */
typedef enum {
	GRAPH_FF_CLIENT_IDENTIFIED	= 0x00000001,
	GRAPH_FF_SESSION_LEADER		= 0x00000002
} graph_file_flags_t;

typedef struct {
	kgic_mapper_mmap_setup_request_t request;
	const kgi_resource_t *resource;
	pid_t pid;
	gid_t gid;
} graph_mmap_setup_t;

struct graph_file_s {
	unsigned long refcnt;
	graph_file_flags_t	flags;
	graph_mmap_setup_t	mmap_setup;
	kgi_u_t			device_id;
	kgi_u_t			previous;
	graph_device_t		*device;
	graph_mapping_t		*mappings;
};

#define	GRAPH_MAX_NR_DEVICES	16
#define	GRAPH_MAX_NR_IMAGES	16

/* Maximum number of opened graph files */
#define GRAPH_MAX_NR_FILES	16

extern graph_file_t *graph_files[GRAPH_MAX_NR_FILES];

extern int graph_command(graph_file_t *file, unsigned int cmd, void *data,
		struct thread *td);
extern int graph_resource_command(graph_file_t *file, unsigned int cmd,
	 	void *data);

extern void graph_unmap_map(graph_mapping_t *map);
extern void graph_unmap_resource(graph_mapping_t *map);

extern void graph_add_mapping(graph_file_t *file, graph_mapping_t *map);
extern void graph_delete_mapping(graph_mapping_t *map);

extern void graph_device_map(kgi_device_t *dev);
extern kgi_s_t graph_device_unmap(kgi_device_t *dev);

#endif	/* _KGI_GRAPHIC_H_ */
