/* $Id: visual.c,v 1.1 2000/02/22 11:12:23 taylor_j Exp $
******************************************************************************

   Display-kgi: initialization

   Copyright (C) 1995 Andreas Beck      [andreas@ggi-project.org]
   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]
   Copyright (C) 2000 Jon Taylor    	[taylorj@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <kgi/kgi.h>
#include <kgi/cmd.h>
#include <ggi/internal/ggi-dl.h>

void _ignore_SIGBUS(int unused)
{ 
	signal(SIGBUS, _ignore_SIGBUS);
	sleep(1);	/* Ignore the SIGBUSes */
}

int GGIdlinit(ggi_visual *vis, const char *args, void *argptr)
{
	LIBGGI_FD(vis)=LIBGGI_SELECT_FD(vis)=open(args,O_RDWR);
	if (LIBGGI_FD(vis)<0)
		return GGI_DL_ERROR;

	/* Has mode management */
	vis->opdisplay->getmode		= GGIgetmode;
	vis->opdisplay->setmode 	= GGIsetmode;
	vis->opdisplay->checkmode 	= GGIcheckmode;
	vis->opdisplay->kgicommand 	= GGIkgicommand;
	vis->opdisplay->setflags 	= GGIsetflags;

	/* Has Event management */
	vis->opdisplay->eventpoll	= GGIeventpoll;
	vis->opdisplay->eventread	= GGIeventread;
	vis->opdisplay->seteventmask	= GGIseteventmask;

//	vis->opdraw->setorigin=GGIsetorigin;

	/* temporary hack to do away with the SIGBUS ... */
	/* FIXME: What is this SIGBUS thing? */
	signal(SIGBUS,_ignore_SIGBUS);

	return GGI_DL_OPDISPLAY|GGI_DL_OPDRAW;
}

int GGIdlcleanup(ggi_visual *vis)
{
	if (LIBGGI_FD(vis) > -1)
	  close(LIBGGI_FD(vis));

	return 0;
}
		
#include <ggi/internal/ggidlinit.h>
