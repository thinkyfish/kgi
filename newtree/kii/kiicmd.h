/* ----------------------------------------------------------------------------
**	KII command definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	
**	$FreeBSD$
**	
*/
#ifndef _kii_command_h
#define	_kii_command_h

/* ----------------------------------------------------------------------------
**	null command data. No input, no output.
** ----------------------------------------------------------------------------
*/
typedef struct
{
} kiic_null_t;

#define	kiic_null_result_t	kiic_null_t
#define	kiic_null_request_t	kiic_null_t

#define KIIC_UNION(type, command)			\
typedef union						\
{ 							\
	kiic_##type##_##command##_request_t request;	\
	kiic_##type##_##command##_result_t  result;	\
} kiic_##type##_##command##_union_t;

/* ----------------------------------------------------------------------------
**	mapper commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	kii_ascii_t	client[64];
	kii_version_t	client_version;

} kiic_mapper_identify_request_t;

typedef struct
{
	kii_ascii_t	mapper[64];
	kii_version_t	mapper_version;

} kiic_mapper_identify_result_t;

KIIC_UNION(mapper, identify);

typedef kiic_null_request_t kiic_mapper_get_keymap_info_request_t;

typedef struct
{
	kii_u_t	fn_buf_size;
	kii_u_t fn_str_size;

	kii_u_t	keymin, keymax;
	kii_u_t keymap_size;

	kii_u_t combine_size;

} kiic_mapper_get_keymap_info_result_t;

KIIC_UNION(mapper, get_keymap_info);

typedef struct
{
	kii_u_t	keymap, keymin, keymax;

} kiic_mapper_get_keymap_request_t, kiic_mapper_set_keymap_result_t;

typedef struct
{
	kii_u_t	keymap, keymin, keymax;
	kii_unicode_t	map[256];

} kiic_mapper_get_keymap_result_t, kiic_mapper_set_keymap_request_t;

KIIC_UNION(mapper, get_keymap);
KIIC_UNION(mapper, set_keymap);

typedef struct {
	kii_u_t device_id;
} kiic_mapper_attach_request_t;

typedef kiic_null_result_t kiic_mapper_attach_result_t;

KIIC_UNION(mapper, attach);

typedef struct {
	kii_u_t unit;
} kiic_mapper_get_unit_request_t;

typedef struct {
	kii_u_t unit;
} kiic_mapper_get_unit_result_t;

KIIC_UNION(mapper, get_unit);

/* ----------------------------------------------------------------------------
**	KII command encoding
** ----------------------------------------------------------------------------
*/

/* Include ioccom.h for _IOxx definitions
 */
#include <sys/ioccom.h>

#define KIIC_IO(type, command, callback, code) \
	KIIC_##type##_##command = _IO(KIIC_##type##_COMMAND >> 8, code)

#define KIIC_IOR(type, command, callback, code) \
	KIIC_##type##_##command = _IOR(KIIC_##type##_COMMAND >> 8, code, kiic_##callback##_result_t)

#define KIIC_IOW(type, command, callback, code) \
	KIIC_##type##_##command = _IOW(KIIC_##type##_COMMAND >> 8, code, kiic_##callback##_request_t)

#define KIIC_IOWR(type, command, callback, code) \
	KIIC_##type##_##command = _IOWR(KIIC_##type##_COMMAND >> 8, code, kiic_##callback##_union_t)
	

typedef enum
{
	KIIC_MAPPER_COMMAND	= 0x00000000,	/* ext. mapper commands	*/
		KIIC_IOR (MAPPER, IDENTIFY,		mapper_identify,	0x0001),
		KIIC_IO  (MAPPER, MAP_DEVICE,		null,			0x0002),
		KIIC_IO  (MAPPER, UNMAP_DEVICE,		null,			0x0003),
		KIIC_IOR (MAPPER, GET_KEYMAP_INFO,	mapper_get_keymap_info,	0x0004),
		KIIC_IOWR(MAPPER, GET_KEYMAP,		mapper_get_keymap,	0x0005),
		KIIC_IOWR(MAPPER, SET_KEYMAP,		mapper_set_keymap,	0x0006),
		KIIC_IOW (MAPPER, ATTACH,		mapper_attach,		0x0007),
		KIIC_IOWR(MAPPER, GET_UNIT,		mapper_get_unit,	0x0008),

	KIIC_TYPE_MASK		= 0x0000FF00,
	KIIC_NR_MASK		= 0x000000FF,
	KIIC_COMMAND_MASK	= KIIC_TYPE_MASK | KIIC_NR_MASK

} kii_command_t;

#define	KIIC_COMMAND(cmd)	((cmd) & KIIC_COMMAND_MASK)
#define	KIIC_SIZE(cmd)		(IOCPARM_LEN(cmd))
#define	KIIC_READ(cmd)		((cmd) & IOC_OUT)
#define	KIIC_WRITE(cmd)		((cmd) & IOC_IN)

#endif	/* #ifndef _kii_command_h */
