/* ----------------------------------------------------------------------------
**	backward compatiblity definitions
** ----------------------------------------------------------------------------
*/

#ifndef	_compatibility_h
#define	_compatibility_h

#define	VENDOR_STRING	"The KGI Project"
#define	VENDOR_RELEASE	1


#ifdef	PIXMAP_PRIVATES
#	define	PIXPRIV
#endif


/* ----------------------------------------------------------------------------
**	extensions
** ----------------------------------------------------------------------------
*/

#ifdef	X11_EXT_DEC_DPMS
#	define	DPMSExtension
/*	#define DPMS_SERVER	*/
#endif

#ifdef	X11_EXT_DEC_XINERAMA
#	define	PANORAMIX
/*	#define	PANORAMIX_DEBUG	*/
#endif



#ifdef	X11_EXT_NCD_LBX
#	define	LBX
/*	LBX_NEED_OLD_SYMBOL_FOR_LOADABLES */
#endif



#ifdef	X11_EXT_TOG_SYNC
#	define	XSYNC
#endif

#ifdef	X11_EXT_TOG_BIGREQUESTS
#	define	BIGREQS
#endif

#ifdef	X11_EXT_TOG_XINPUT
#	define	XINPUT
#endif

#ifdef	X11_EXT_TOG_XTEST
#	define	XTESTEXT1
#endif



#ifdef	X11_EXT_SGI_MISC
#	define	SGIMISC
#endif

#ifdef	X11_EXT_SGI_XKB
#	define	XKB
#endif




#ifdef	X11_EXT_XC_APPGROUP
#	define	XAPPGROUP
#endif

#ifdef	X11_EXT_XC_SECURITY
#	define	XCSECURITY
#endif

#ifdef	X11_EXT_XC_XPRINT
#	define	XPRINT
#endif



#ifdef	X11_EXT_XFREE86_BIGFONT
#	define	XF86BIGFONT
#endif


/* ----------------------------------------------------------------------------
**	connections
** ----------------------------------------------------------------------------
*/

#ifdef	X11_CONN_AMTCP
#	define	AMTCPCONN
#endif

#ifdef	X11_CONN_AMRPC
#	define	AMRPCCONN
#endif

#ifdef	X11_CONN_CHAOS
#	define	CHAOSCONN
#endif

#ifdef	X11_CONN_DNET
#	define	DNETCONN
#endif

#ifdef	X11_CONN_LOCAL
#	define	LOCALCONN
#endif

#ifdef	X11_CONN_TCP
#	define	TCPCONN
#endif

#ifdef	X11_CONN_UNIX
#	define	UNIXCONN
#endif

#ifdef	X11_CONN_STREAMS
#	define	STREAMSCONN
#endif

/*	not yet supported/used:
**
**	MNX_TCPCONN
**	OS2PIPECONN
**
**	WINTCP
**	BSD44SOCKETS
*/


/*	flags
**
**	SMART_DEBUG
**	SMART_SCHEDULE
**	SMART_SCHEDULE_POSSIBLE
**	X_NOT_STDC_ENV
**	NOSTDHDRS
**	X_NOT_POSIX
**	X_POSIX_C_SOURCE
**	XNO_SYSCONF
**	K5AUTH
**	MAXPATHLEN
**	PATH_MAX
**	SECURE_RPC
**	VARIABLE_IFREQ
**	HAS_IFREQ
**	HAS_GETDTABLESIZE
**	RLIMIT_DATA
**	RLIMIT_NOFILE
**	RLIMIT_STACK
**	ADMPATH
**	SHAPE
**
**	XDMCP
**	HASXDMAUTH
**	_SC_OPEN_MAX
**	STACK_DIRECTION
**	ALIGN_SIZE
**	XDEBUG
**	DEBUG
**	FONTDEBUG
**	USE_RGB_TXT	- use RGB text database
**	NDBM	- has new dbm library if defined
**	DDXOSINIT
**	SERVER_LOCK
**	XFree86LOADER
**	FATALERRORS
**	COMMANDLINE_CHALLENGED_OPERATING_SYSTEMS
**	DDXOSFATALERROR
**	DDXOSVERRORF
**	DDXTIME
**	ABORTONFATALERROR
**	NOLOGOHACK
**	GPROF
**	INTERNAL_MALLOC
**	SPECIAL_MALLOC
**	XFREE_ERASES
**	XALLOC_DEBUG
**	XALLOC_LOG
**	SIZE_TAIL
**	PAGE_SIZE
**	MMAP_DEV_ZERO
**	INTERNAL_MALLOC
**	HAS_GETPAGESIZE
**	HAS_SC_PAGESIZE
**	HAS_MMAP_ANON
**	USE_CHMOD
**	SHAPE
**	NEED_SCREEN_REGIONS
**	NEED_DBE_BUF_BITS
**	DO_SAVE_UNDERS
*/

/*	platforms
**
**	AMOEBA
**	MINIX
**	_MINIX
**	WIN32
**	DGUX
**	sun
**	sgi
**	SUNSYSV
**	i386
**	ultrix
**	linux
**	hpux
**	hpux_not_tog
**	apollo
**	macII
**	__QNXNTO__
**	__GNU__
**	__EMX__
**	Lynx
**	NCR
**	SCO
**	QNX4
**	SCO325
**	SVR4
**	SYSV
**	ESIX
**	ISC
**	CSRG_BASED
**	AIXV3
**	AIXrt
**	AIX386
*/

#endif	/* #ifdef _compatibility_h	*/
