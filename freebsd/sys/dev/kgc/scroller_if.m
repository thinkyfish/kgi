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
# KGC scroller interface
#

#include <dev/kgi/system.h>
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_scroller.h>

INTERFACE scroller;

METHOD kgi_s_t init {
	scroller_t s;
	kgi_u16_t *buffer;
};

METHOD void done {
	scroller_t s;
};

METHOD void get {
	scroller_t s;
	kgi_ucoord_t *size;
	kgi_u_t *top;
	kgi_u_t *bottom;
	kgi_u_t *x;
	kgi_u_t *y;
	kgi_u_t *attrfl;
	kgi_u_t *erase;
};

METHOD void set {
	scroller_t s;
	kgi_u_t attrfl;
	kgi_u_t erase;
};

METHOD void margins {
	scroller_t s;
	kgi_u_t top;
	kgi_u_t bottom;
};

METHOD void map {
	scroller_t s;
};

METHOD void unmap {
	scroller_t s;
};

METHOD void reset {
	scroller_t s;
};

METHOD void erase_display {
	scroller_t s;
	kgi_u_t arg;
};

METHOD void gotoxy {
	scroller_t s;
	kgi_s_t x;
	kgi_s_t y;
};

METHOD void backward {
	scroller_t s;
	kgi_s_t lines;
};

METHOD void forward {
	scroller_t s;
	kgi_s_t lines;
};

METHOD void update_attr {
	scroller_t s;
};

METHOD void bs {
	scroller_t s;
};

METHOD void hts {
	scroller_t s;
};

METHOD void tbc {
	scroller_t s;
	kgi_u_t tab;
};

METHOD void ht {
	scroller_t s;
};

METHOD void lf {
	scroller_t s;
};

METHOD void cr {
	scroller_t s;
};

METHOD void sync {
	scroller_t s;
};

METHOD void write {
	scroller_t s;
	kgi_isochar_t c;
};

METHOD void mark {
	scroller_t s;
};

METHOD void modified_mark {
	scroller_t s;
};

METHOD void modified_wrap {
	scroller_t s;
};

METHOD void mksound {
	scroller_t s;
	kgi_u_t pitch;
	kgi_u_t duration;
};

METHOD void erase_line {
	scroller_t s;
	kgi_u_t arg;
};

METHOD void insert_chars {
	scroller_t s;
	kgi_u_t n;
};

METHOD void delete_chars {
	scroller_t s;
	kgi_u_t n;
};

METHOD void erase_chars {
	scroller_t s;
	kgi_u_t n;
};

METHOD void reverse_lf {
	scroller_t s;
};

METHOD void down {
	scroller_t s;
	kgi_u_t t;
	kgi_u_t b;
	kgi_u_t n;
};

METHOD void up {
	scroller_t s;
	kgi_u_t t;
	kgi_u_t b;
	kgi_u_t n;
};

METHOD void move {
	scroller_t s;
	kgi_s_t c;
	kgi_s_t l;
};

METHOD void scroll_top {
	scroller_t s;
	kgi_u_t n;
};

METHOD void scroll_bottom {
	scroller_t s;
	kgi_u_t n;
};

METHOD void gotox {
	scroller_t s;
	kgi_s_t x;
};

METHOD void gotoy {
	scroller_t s;
	kgi_s_t y;
};

METHOD void save {
	scroller_t s;
};

METHOD void restore {
	scroller_t s;
};
