#include <kgi/module.h>
#include "monosync-meta.h"

#define	MONOSYNC_MONITOR_VGA_HTIMINGS	1
static const kgim_monitor_timing_t monosync_monitor_VGA_htiming[MONOSYNC_MONITOR_VGA_HTIMINGS] =
{
/* 31.468 */	{  25422,  25422,  27011,  30824,  31142,  31778, 0 }
};

#define	MONOSYNC_MONITOR_VGA_VTIMINGS	3
static const kgim_monitor_timing_t monosync_monitor_VGA_vtiming[MONOSYNC_MONITOR_VGA_VTIMINGS] =
{
/* 60.1 */	{   480,   488,   489,   491,   516,   524, MONOSYNC_HNVN | 0 },
/* 69.9 */	{   400,   407,   412,   414,   442,   450, MONOSYNC_HNVP | 0 },
/* 70.1 */	{   350,   356,   387,   389,   443,   449, MONOSYNC_HPVN | 0 }
};

const monosync_timing_t monosync_monitor_timing =
{
	"VGA",
	MONOSYNC_MONITOR_VGA_HTIMINGS,
	monosync_monitor_VGA_htiming,
	MONOSYNC_MONITOR_VGA_VTIMINGS,
	monosync_monitor_VGA_vtiming,
};
