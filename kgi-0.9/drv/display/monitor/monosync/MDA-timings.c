#include <kgi/module.h>
#include "monosync-meta.h"

#define	MONOSYNC_MONITOR_MDA_HTIMINGS	1
static const kgim_monitor_timing_t monosync_monitor_MDA_htiming[MONOSYNC_MONITOR_MDA_HTIMINGS] =
{
/* 18.622 */	{  44288,  44288,  45400,  53700,  53700,  53700, 0 }
};

#define	MONOSYNC_MONITOR_MDA_VTIMINGS	2
static const kgim_monitor_timing_t monosync_monitor_MDA_vtiming[MONOSYNC_MONITOR_MDA_VTIMINGS] =
{
/* 50.3 */	{   348,   348,   352,   370,   370,   370, MONOSYNC_HPVN | 0 },
/* 50.3 */	{   350,   350,   364,   370,   370,   370, MONOSYNC_HNVP | 0 }
};

const monosync_timing_t monosync_monitor_timing =
{
	"MDA",
	MONOSYNC_MONITOR_MDA_HTIMINGS,
	monosync_monitor_MDA_htiming,
	MONOSYNC_MONITOR_MDA_VTIMINGS,
	monosync_monitor_MDA_vtiming,
};
