/*-
 * Copyright (c) 1995-1998 Søren Schmidt
 * All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Sascha Wildner <saw@online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Last synconrized with FreeBSD -r189617:
 * head/sys/dev/syscons/syscons.h 
 */

#ifndef _DEV_SYSCONS_SYSCONS_H_
#define	_DEV_SYSCONS_SYSCONS_H_

#include <sys/lock.h>
#include <sys/mutex.h>

#ifndef MAXCONS
#define MAXCONS	24
#endif

#define PCBURST 128
#define SCEVT_COLD 0
#define SCEVT_WARM 1

typedef struct {
	union {	/* keep it first */
		kgi_console_t any;
		kgi_console_dumb_t dumb;
		kgi_console_xterm_t xterm;
	} type;
} sce_console;

typedef struct sce_ttysoftc {
	int unit;
} sce_ttysoftc;

extern sce_console *sce_consoles[CONFIG_KGII_MAX_NR_CONSOLES];

extern int sce_mouse_init(void);
extern int sce_sysmouse_event(kii_event_t *ev);
extern int sce_sysmouse_init(void);
extern int sce_sysmouse_ioctl(struct tty *tp, u_long cmd, caddr_t data, 
		struct thread *td);

#endif /* !_DEV_SYSCONS_SYSCONS_H_ */
