#!/usr/bin/awk -f
# -----------------------------------------------------------------------------
#	AWK script to convert XF86Config Modelines to KGI timelist timing file
# -----------------------------------------------------------------------------
#	Copyright (C)	1999	Steffen Seeger
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#
# -----------------------------------------------------------------------------
#
#	command line usage:
#		./xfree2t.awk <XF86Config>
#
#	$Log: xfree2t.awk,v $
#	

/Modeline/ {
	dclk = $3;
	xtotal = $7;
	ytotal = $11;
	vfreq = 1e6 * dclk / (xtotal * ytotal);
	hfreq = 1e3 * dclk / xtotal;
	printf "\n# XFree %s, %.1f Hz, %.3f kHz\n", $2, vfreq, hfreq
	printf "dclk\t%.3f\n", $3

	if (index($0, "Interlace"))	printf("interlaced\n");
	if (index($0, "+hsync"))	printf("hsync\t+\n");
	if (index($0, "-hsync"))	printf("hsync\t-\n");
	if (index($0, "+vsync"))	printf("vsync\t+\n");
	if (index($0, "-vsync"))	printf("vsync\t-\n");

	printf "htiming\t%i\t%i\t%i\t%i\t%i\t%i\n", $4, $4, $5, $6, $7, $7
	printf "vtiming\t%i\t%i\t%i\t%i\t%i\t%i\n", $8, $8, $9, $10, $11, $11
}
