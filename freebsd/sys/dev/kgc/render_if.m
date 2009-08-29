#-
# Copyright (C) 2003 Nicholas Souchu
#
# This file is distributed under the terms and conditions of the 
# MIT/X public license. Please see the file COPYRIGHT.MIT included
# with this software for details of these terms and conditions.
# Alternatively you may distribute this file under the terms and
# conditions of the GNU General Public License. Please see the file 
# COPYRIGHT.GPL included with this software for details of these terms
# and conditions.

#
# KGC renderer interface
#

#include <dev/kgi/system.h>
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_render.h>

INTERFACE render;

METHOD kgi_s_t init {
	render_t r;
	kgi_u_t devid;
};

METHOD void done {
	render_t r;
};

METHOD void map {
	render_t r;
};

METHOD void unmap {
	render_t r;
};

METHOD void get {
	render_t r;
	kgi_ucoord_t *size;
	kgi_ucoord_t *virt;
	kgi_u_t *flags;
};

METHOD void set {
	render_t r;
	kgi_ucoord_t *size;
	kgi_ucoord_t *virt;
};

METHOD int put_text {
	render_t r;
	kgc_textbuf_t *tb;
	kgi_u_t start;
	kgi_u_t offset;
	kgi_u_t count;
};

METHOD kgi_u_t atop {
	render_t r;
	kgi_u_t attr;
};

METHOD kgi_u_t ptoa {
	render_t r;
	kgi_u_t val;
};

METHOD kgi_isochar_t ptoc {
	render_t r;
	kgi_u_t pos;
};

METHOD kgi_u_t ctop {
	render_t r;
	kgi_isochar_t sym;
};

METHOD void hide_gadgets {
	render_t r;
};

METHOD void undo_gadgets {
	render_t r;
};

METHOD void show_gadgets {
	render_t r;
	kgi_u_t x;
	kgi_u_t y;
	kgi_u_t offset;
};

METHOD void load_font {
	render_t r;
	kgi_u_t page;
	kgi_u_t size;
	kgi_u8_t *data;
	kgi_u_t ch;
	kgi_s_t count;
};

METHOD void save_font {
	render_t r;
	kgi_u_t page;
	kgi_u_t size;
	kgi_u8_t *data;
	kgi_u_t ch;
	kgi_s_t count;
};

METHOD void show_font {
	render_t r;
	kgi_u_t page;
};
