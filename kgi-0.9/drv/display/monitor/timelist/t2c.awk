#!/usr/bin/awk -f
# -----------------------------------------------------------------------------
#	AWK script to convert timelist timing file into timelist timing C code
# -----------------------------------------------------------------------------
#	Copyright (C)	2001		Brian S. Julin
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#
# -----------------------------------------------------------------------------
#
#	command-line usage:	
#		t2c.awk timelist=<file-base-name> <file-name>
#
/htiming/ {

	xwidth[htimings]  = $2;
	xbstart[htimings] = $3;
	xsstart[htimings] = $4;
	xsend[htimings]   = $5;
	xbend[htimings]   = $6;
	xtotal[htimings]  = $7;
	htimings++;
	xwidth[htimings]  = 0;
	xbstart[htimings] = 0;
	xsstart[htimings] = 0;
	xsend[htimings]   = 0;
	xbend[htimings]   = 0;
	xtotal[htimings]  = 0;
}

/hsync/ {
	if ($2 == "+") { hsync[vtimings] = "HP"; }
	if ($2 == "-") { hsync[vtimings] = "HN"; }
}

/vsync/ {
	if ($2 == "+") { vsync[vtimings] = "VP"; }
	if ($2 == "-") { vsync[vtimings] = "VN"; }
}

/dclk/ {
	dclk[vtimings]  = $2;
}

/vtiming/ {

	ywidth[vtimings]  = $2;
	ybstart[vtimings] = $3;
	ysstart[vtimings] = $4;
	ysend[vtimings]   = $5;
	ybend[vtimings]   = $6;
	ytotal[vtimings]  = $7;
	htiming[vtimings] = htimings-1;

	vtimings++;

	ywidth[vtimings]  = 0;
	ybstart[vtimings] = 0;
	ysstart[vtimings] = 0;
	ysend[vtimings]   = 0;
	ybend[vtimings]   = 0;
	ytotal[vtimings]  = 0;
	hsync[vtimings]  = "HP";
	vsync[vtimings]  = "VP";
}

BEGIN {

	htimings = 0;
	vtimings = 0;
}

END {
	if (htimings && vtimings) {

		printf("#include <kgi/module.h>\n");
		printf("#include \"timelist-meta.h\"\n\n");

		printf("#define\tTIMELIST_MONITOR_%s_HTIMINGS\t%i\n",
			timelist, htimings);
		printf("static const struct {\n");
		printf("\tkgi_u_t\tdclk;\n");
		printf("\tkgim_monitor_timing_t tm;\n\t} timelist_monitor_");
		printf("%s_htiming[TIMELIST_MONITOR_%s_HTIMINGS] =\n{\n",
			timelist, timelist);
		for (i = 0; i < htimings; i++) {

			printf("{%d, \t{ %6i, %6i, %6i, %6i, %6i, %6i, %i }}",
				1e6 * dclk[i],
				xwidth[i], xbstart[i], xsstart[i],
				xsend[i], xbend[i], xtotal[i], i);
			printf((i < htimings-1) ? ",\n" : "\n");
		}
		printf("};\n");


		printf("\n#define\tTIMELIST_MONITOR_%s_VTIMINGS\t%i\n",
			timelist, vtimings);
		printf("static const kgim_monitor_timing_t timelist_monitor_");
		printf("%s_vtiming[TIMELIST_MONITOR_%s_VTIMINGS] =\n{\n",
			timelist, timelist);
		for (i = 0; i < vtimings; i++) {

			printf("/* %.1f */\t{ %5i, %5i, %5i, %5i, %5i, %5i, TIMELIST_%s%s | %i }",
				(1e6 * dclk[i])/(xtotal[i]*ytotal[i]),
				ywidth[i], ybstart[i], ysstart[i],
				ysend[i], ybend[i], ytotal[i],
				hsync[i], vsync[i], htiming[i]);
			printf((i < vtimings-1) ? ",\n" : "\n");
		}
		printf("};\n");

		printf("\nconst timelist_timing_t timelist_monitor_timing =\n{\n",
			timelist);
		printf("\t\"%s\",\n", timelist);
		printf("\tTIMELIST_MONITOR_%s_HTIMINGS,\n", timelist);
		printf("\ttimelist_monitor_%s_htiming,\n", timelist);
		printf("\tTIMELIST_MONITOR_%s_VTIMINGS,\n", timelist);
		printf("\ttimelist_monitor_%s_vtiming,\n};\n", timelist);
	}
}

