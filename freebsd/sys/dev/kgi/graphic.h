/*-
 * Copyright (C) 1998-2000 Steffen Seeger
 *
 * This file is distributed under the terms and conditions of the 
 * MIT/X public license. Please see the file COPYRIGHT.MIT included
 * with this software for details of these terms and conditions.
 * Alternatively you may distribute this file under the terms and
 * conditions of the GNU General Public License. Please see the file 
 * COPYRIGHT.GPL included with this software for details of these terms
 * and conditions.
 */

/*
 * /dev/graphic resource mapper definition.
 */
#ifndef _kgi_graphic_h
#define	_kgi_graphic_h

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
		
	kgi_u_t		device_id;
	kgi_u_t		previous;

	graph_device_t	*device;
	graph_mapping_t	*mappings;
};

#define	GRAPH_MAX_NR_DEVICES	16
#define	GRAPH_MAX_NR_IMAGES		16

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

#endif	/* #ifndef _kgi_graphic_h */
