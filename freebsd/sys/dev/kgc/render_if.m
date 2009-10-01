#-
# Copyright (c) 2003 Nicholas Souchu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and permit to persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#   
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,EXPRESSED OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
# AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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
