/* ---------------------------------------------------------------------------
**      FreeBSD KII kbd driver header
** ---------------------------------------------------------------------------
**      Copyright (C)   2003       Nicolas Souchu
**
**      This file is distributed under the terms and conditions of the
**      MIT/X public license. Please see the file COPYRIGHT.MIT included
**      with this software for details of these terms and conditions.
**      Alternatively you may distribute this file under the terms and
**      conditions of the GNU General Public License. Please see the file
**      COPYRIGHT.GPL included with this software for details of these terms
**      and conditions.
** -------------------------------------------------------------------------
**
**      $FreeBSD$
**
*/
#ifndef _DEV_KGI_KIP_H
#define _DEV_KGI_KIP_H

/* Enable KII registration of a kbd freshly registered */
void kip_kbd_register(keyboard_t *kbd, int index);

#endif /* !_DEV_KGI_KIP_H */
