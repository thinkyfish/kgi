#include <kgi/module.h>
#include "monosync-meta.h"

#define	MONOSYNC_MONITOR_SVGA_HTIMINGS	3
static const kgim_monitor_timing_t monosync_monitor_SVGA_htiming[MONOSYNC_MONITOR_SVGA_HTIMINGS] =
{
/* 56.475 */	{  13653,  13653,  13973,  15786,  17707,  17707, 0 },
/* 48.077 */	{  16000,  16000,  17120,  19520,  20800,  20800, 1 },
/* 31.468 */	{  25422,  25422,  27011,  30824,  31142,  31778, 2 }
};

#define	MONOSYNC_MONITOR_SVGA_VTIMINGS	5
static const kgim_monitor_timing_t monosync_monitor_SVGA_vtiming[MONOSYNC_MONITOR_SVGA_VTIMINGS] =
{
/* 70.1 */	{   768,   768,   771,   777,   806,   806, MONOSYNC_HNVN | 0 },
/* 72.2 */	{   600,   600,   637,   643,   666,   666, MONOSYNC_HNVP | 1 },
/* 60.1 */	{   480,   488,   489,   491,   516,   524, MONOSYNC_HNVN | 2 },
/* 69.9 */	{   400,   407,   412,   414,   442,   450, MONOSYNC_HNVP | 2 },
/* 70.1 */	{   350,   356,   387,   389,   443,   449, MONOSYNC_HPVN | 2 }
};

const monosync_timing_t monosync_monitor_timing =
{
	"SVGA",
	MONOSYNC_MONITOR_SVGA_HTIMINGS,
	monosync_monitor_SVGA_htiming,
	MONOSYNC_MONITOR_SVGA_VTIMINGS,
	monosync_monitor_SVGA_vtiming,
};
