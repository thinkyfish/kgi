#
# Makefile for the KGI sample implementation and device drivers.
#
# This file is distributed under the terms and conditions of the 
# MIT/X public license. Please see the file COPYRIGHT.MIT included
# with this software for details of these terms and conditions.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now inherited from the
# parent makes..
#

SUB_DIRS     := 
MOD_SUB_DIRS := $(SUB_DIRS)
ALL_SUB_DIRS := $(SUB_DIRS)

L_TARGET := kgi.a
M_OBJS   :=
L_OBJS   := console.o default.o
LX_OBJS  := kgi.o kii.o system.o

ifeq ($(CONFIG_KGI_VT_LINUX),y)
ifeq ($(CONFIG_INPUT),y)
	L_OBJS += vt.o
else
	L_OBJS += vt.o psaux.o
endif
endif

ifeq ($(CONFIG_KGI_DEV_GRAPHIC),y)
	L_OBJS += graphic.o
else
	ifeq ($(CONFIG_KGI_DEV_GRAPHIC),m)
		M_OBJS += graphic.o
	endif
endif

ifeq ($(CONFIG_KGI_DEV_EVENT),y)
	L_OBJS := $(L_OBJS)
endif

ifeq ($(CONFIG_INPUT),y)
	L_OBJS   += input.o
else
	L_OBJS   += kbd-i386.o
endif

ifeq ($(CONFIG_KGI_DPY_NULL),y)
	L_OBJS += dpy-null.o
endif

ifeq ($(CONFIG_KGI_DPY_I386), y)
	L_OBJS   := $(L_OBJS) dpy-i386.o
endif

ifeq ($(CONFIG_KGI_TERM_XTERM),y)
	L_OBJS   += xterm.o
endif

include $(TOPDIR)/Rules.make
