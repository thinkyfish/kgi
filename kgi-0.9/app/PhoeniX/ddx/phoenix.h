/* ----------------------------------------------------------------------------
**	PhoeniX internal structures & data types
** ----------------------------------------------------------------------------
**
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: phoenix.h,v $
**	Revision 1.1  2000/09/21 11:21:04  seeger_s
**	- added phoenix global definitions
**	
*/
#ifndef	_ddx_phoenix_h
#define	_ddx_phoenix_h

#include <kgi/kgi.h>
#include <kii/kii.h>

typedef enum
{
	PHOENIX_F_INIT_OUTPUT_HW	= 0x00000001,
	PHOENIX_F_INIT_INPUT_HW		= 0x00000002

} phoenix_flags_t;


typedef struct
{
	ScreenPtr		x11;	/* server visible data		*/

	xf86ScreenPtr		xf86;	/* XFree86 compatibility data	*/
	XAAInfoRecPtr		xaa;	/* XAA info rec pointer		*/

	/*	these need to be configured
	*/
	kgi_image_mode_t	img;	/* image mode for that screen	*/

	/*	these are read from the device driver
	*/
	ucoord_t		dpi;	/* dots per inch		*/
	u_t			width;	/* width in mm			*/

	void			*fb;	/* pointer to frame buffer	*/

} phoenix_output_t;

typedef struct
{
	u_t			enabled;	/* # of enabled devices	*/
	kii_context_t		*kii;		/* kii context pointer	*/

} phoenix_input_t;

typedef struct
{
	phoenix_flags_t		flags;

	u_t			outputs;
	phoenix_output_t	output[MAXSCREENS];

	phoenix_input_t		input;
	HWEventQueueType	always_check[2];

} phoenix_t;

extern phoenix_t phoenix;

#endif	/* ifndef _ddx_phoenix_h	*/
