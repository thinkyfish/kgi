# -----------------------------------------------------------------------------
#	Linux version detection and patch selection
# -----------------------------------------------------------------------------
#
#	Copyright (C)	2000	Steffen Seeger
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#
#	$Log: version.awk,v $
#	Revision 1.3  2000/04/17 16:05:01  seeger_s
#	- added copyright notice
#	
#	Revision 1.2  2000/02/22 11:05:29  taylor_j
#	- added 2.2.14 support
#	
#

/^VERSION =/      { LINUX_MAJOR=$3; printf "LINUX_MAJOR=%s;\n", LINUX_MAJOR }
/^PATCHLEVEL =/   { LINUX_MINOR=$3; printf "LINUX_MINOR=%s;\n", LINUX_MINOR }
/^SUBLEVEL =/     { LINUX_PATCH=$3; printf "LINUX_PATCH=%s;\n", LINUX_PATCH }
/^EXTRAVERSION =/ { LINUX_EXTRA=$3; printf "LINUX_EXTRA=%s;\n", LINUX_EXRTA }

END {
	if ((LINUX_MAJOR < 2) || 
		((LINUX_MAJOR == 2) && (LINUX_MINOR <  2))) {

		printf "PATCH=notanylonger\n"
		exit 0

	}

	if ((LINUX_MAJOR > 2) ||
		((LINUX_MAJOR == 2) && (LINUX_MINOR >  2))) {

		printf	"PATCH=notyet\n"
		exit 0

	}

	PATCH = "2.2.0"
	PMAKE = "2.2.0"

	if (LINUX_PATCH >=  3) {			PMAKE="2.2.3";  }
	if (LINUX_PATCH >=  4) {			PMAKE="2.2.4";  }
	if (LINUX_PATCH >=  7) { 			PMAKE="2.2.7";  }
	if (LINUX_PATCH >= 12) { PATCH="2.2.12";	PMAKE="2.2.12"; }
	if (LINUX_PATCH >= 13) { PATCH="2.2.13";	                }
	if (LINUX_PATCH >= 14) { PATCH="2.2.14";	PMAKE="2.2.14"; }

	if (LINUX_PATCH == 8) { 

		printf  "\n\n\tWARNING: linux-2.2.8 has file system " \
			"corruption problems!\n" > "/dev/stderr"
		printf	"\tconfiguration process aborted.\n\n\n" > "/dev/stderr"
		printf  "exit 1;\n"
	}

	printf "LINUX_VERSION=%s.%s.%s%s;\n",
			LINUX_MAJOR, LINUX_MINOR, LINUX_PATCH, LINUX_EXTRA
	printf "PATCH=%s;\nPMAKE=%s;\n", PATCH, PMAKE
}
