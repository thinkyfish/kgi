/*-
 * Copyright (c) 1997-2000 Steffen Seeger
 * Copyright (c) 2003-2004 Nicholas Souchu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and permit to persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,EXPRESSED OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * A generic debugging framework.
 */

#ifndef	__kgi_kgidbg_h
#define	__kgi_kgidbg_h

/*
 * kernel debugging macros
 */
#ifdef	_KERNEL

/*
 * platform-independent interface definitions
 */

#define	KRN_DEBUG_ANSI_CPP	1

/* Distinguish a bit KGI debug level symbol */
#ifndef KGI_DBG_LEVEL
#define KGI_DBG_LEVEL 0
#endif

/* Ensure compat with KGI original debug level symbol */
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

extern int console_initialized;

#define	KRN_ERROR(fmt, args...) 					\
		if (console_initialized)				\
			printf(fmt "\n", ##args)
#define	KRN_NOTICE(fmt, args...) 					\
		if (console_initialized)				\
			printf(fmt "\n", ##args)
#define	KRN_DEBUG(level, fmt, args...)					\
		if (console_initialized && (level <= KGI_DBG_LEVEL ||   \
		    level <= DEBUG_LEVEL))				\
			printf("<%i> " fmt "\n", level, ##args)
#define KRN_BOOT(fmt, args...)						\
		if (console_initialized && bootverbose)			\
			printf(fmt "\n", ##args)

#define	KRN_ASSERT(x)							\
	if (x) {} else { KRN_DEBUG(0, "%s", #x); }

#define	KRN_INTERNAL_ERROR						\
		KRN_ERROR("KGI internal error")

#ifdef KGI_DBG_LEVEL
#define	KRN_TRACE(level, x)	\
		if (level <= KGI_DBG_LEVEL) { x; } else do {} while (0)
#else
#define	KRN_TRACE(level, x)	
#endif

#ifndef PANIC
#define PANIC(x) panic(__FILE__ ": " x);
#endif

#	define	__krn_error	printf
#	define	__krn_notice	printf
#	define	__krn_debug	log

#else	/* #ifdef _KERNEL */

/*
 * library debugging macros
 */
#ifdef	__LIBRARY__

#	ifdef __STRICT_ANSI__

#		define	LIB_DEBUG_ANSI_CPP	1

#		define	LIB_ERROR	__lib_error
#		define	LIB_DEBUG	__lib_debug
#		define	LIB_NOTICE	__lib_notice

#		define	LIB_ASSERT(x)	if (x) {} else {}

#	else

#		define	LIB_DEBUG_GNU_CPP	1

#		define	LIB_ERROR(fmt, args...)	\
			__lib_error( __FILE__ , __LINE__ , \
				__PRETTY_FUNCTION__ , fmt , ##args )

#		ifdef KGI_DBG_LEVEL
#			define	LIB_DEBUG(level, fmt, args... )		\
				if (level <= KGI_DBG_LEVEL) {		\
					__lib_debug( __FILE__ ,		\
					__LINE__ , __PRETTY_FUNCTION__	\
					, level , fmt , ##args );	\
				}
#		else
#			define	LIB_DEBUG(x...) do {} while(0)
#		endif

#		define	LIB_NOTICE(fmt, args...)		\
			__lib_notice( fmt , ##args )
#		define	LIB_ASSERT(x)	\
			if (x) {} else { LIB_DEBUG(0, "assertion %s failed.", #x); }
#	endif

#	define	LIB_INTERNAL_ERROR	\
		LIB_ERROR("internal error. Please report to %s. Thanks.", MAINTAINER)

#ifdef KGI_DBG_LEVEL
#	define	LIB_TRACE(level, x)	\
		if (level <= KGI_DBG_LEVEL) { x; } else do {} while (0)
#else
#	define	LIB_TRACE(level, x)	
#endif

#endif	/* #ifdef __LIBRARY__ */

/*
 * application debugging macros
 */

#ifdef	__APPLICATION__

#	ifdef __STRICT_ANSI__

#		define	APP_DEBUG_ANSI_CPP	1

#		define	APP_ERROR	__app_error
#		define	APP_DEBUG	__app_debug
#		define	APP_NOTICE	__app_notice

#		define	APP_ASSERT(x)	if (x) {} else {}

#	else

#		define	APP_DEBUG_GNU_CPP	1

#		define	APP_ERROR(fmt, args...)	\
			__app_error( __FILE__ , __LINE__ , \
				__PRETTY_FUNCTION__ , fmt , ##args )

#		ifdef KGI_DBG_LEVEL
#			define	APP_DEBUG(level, fmt, args... )		\
				if (level <= KGI_DBG_LEVEL) {		\
					__app_debug( __FILE__ ,		\
					__LINE__ , __PRETTY_FUNCTION__	\
					, level , fmt , ##args );	\
				}
#		else
#			define	APP_DEBUG(x...) do {} while(0)
#		endif

#		define	APP_NOTICE(fmt, args...)		\
			__app_notice( fmt , ##args )
#		define	APP_ASSERT(x)	\
			if (x) {} else { APP_DEBUG(0, "assertion %s failed.", #x); }
#	endif

#	define	APP_INTERNAL_ERROR	\
		APP_ERROR("internal error. Please report to %s. Thanks.", MAINTAINER)

#ifdef KGI_DBG_LEVEL
#	define	APP_TRACE(level, x)	\
		if (level <= KGI_DBG_LEVEL) { x; } else do {} while (0)
#else
#	define	APP_TRACE(level, x)	
#endif

#endif	/* #ifdef __APPLICATION__ */
#endif	/* #ifdef _KERNEL */

#endif /* _kgi_kgidbg_h */
