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
#	Revision 1.2  2001/09/08 20:56:11  skids
#	
#	Handle multiline modelines.
#	
#	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
#	- initial import of pre-SourceForge tree
#	
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
	printf "htiming\t%i\t%i\t%i\t%i\t%i\t%i\n", $4, $4, $5, $6, $7, $7

	if (index($0, "+vsync"))	printf("vsync\t+\n");
	if (index($0, "-vsync"))	printf("vsync\t-\n");
	printf "vtiming\t%i\t%i\t%i\t%i\t%i\t%i\n", $8, $8, $9, $10, $11, $11
}

/Mode "/ {
    inmode = 1;
}

/DotClock/ {
    if (inmode) dclk = $2;
}

/HTimings/ {

    if (inmode) {
        x1 = $2;
        x2 = $3;
        x3 = $4;
        xtotal = $5;        
    };
}

/VTimings/ {
    if (inmode) {
        y1 = $2;
        y2 = $3;
        y3 = $4;
        ytotal = $5;        
    };
}

/Flags/ {
    if (inmode) {
	if (index($0, "Interlace"))	sync = sync "interlaced\n";
	if (index($0, "+hsync"))	hsync = "hsync\t+\n";
	if (index($0, "+HSync"))	hsync = "hsync\t+\n";
	if (index($0, "-hsync"))	hsync = "hsync\t-\n";
	if (index($0, "-HSync"))	hsync = "hsync\t-\n";
	if (index($0, "+vsync"))	vsync = "vsync\t+\n";
	if (index($0, "+VSync"))	vsync = "vsync\t+\n";
	if (index($0, "-vsync"))	vsync = "vsync\t-\n";
	if (index($0, "-VSync"))	vsync = "vsync\t-\n";
    }
}


/EndMode/ {
	if (inmode) {
	vfreq = 1e6 * dclk / (xtotal * ytotal);
	hfreq = 1e3 * dclk / xtotal;
	printf "\n# XFree %s, %.1f Hz, %.3f kHz\n", $2, vfreq, hfreq
	printf "dclk\t%.3f\n", dclk
	printf "%s", sync
	printf "%s", hsync
	printf "htiming\t%i\t%i\t%i\t%i\t%i\t%i\n",x1,x1,x2,x3, xtotal, xtotal
	printf "%s", vsync
        printf "vtiming\t%i\t%i\t%i\t%i\t%i\t%i\n",y1,y1,y2,y3, ytotal,ytotal
        }

	inmode = 0;
}

